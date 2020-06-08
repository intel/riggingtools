#include <stdexcept>
#include <algorithm>
#include <limits.h>
#include <cstring>
#include <ipp.h>
#include "Smooth_lpfIpp.hpp"

#define check_sts(st) if((st) != ippStsNoErr) { char s[ 100 ]; snprintf( s, 100, "IPP error: %d", (int)st ); throw std::runtime_error(s); }

Smooth_lpfIpp::Smooth_lpfIpp( Smooth_lpfIpp && move )
{
   _delayLine = move._delayLine;
   _newSamples = move._newSamples;
   _firstSampleTimestamp = move._firstSampleTimestamp;
   _internalFirStructureIPP = move._internalFirStructureIPP;
   _ippInternalBuffer1 = move._ippInternalBuffer1;
   _ippInternalBuffer2 = move._ippInternalBuffer2;
   _numTaps = move._numTaps;
   _minNumSamples = move._minNumSamples;
   
   // Reset these on the old object
   move._internalFirStructureIPP = nullptr;
   move._ippInternalBuffer1 = nullptr;
   move._ippInternalBuffer2 = nullptr;
}
Smooth_lpfIpp::~Smooth_lpfIpp()
{
   Uninitialize();
}

void Smooth_lpfIpp::Initialize( int numTaps,
   double normalizedFrequency )
{
   // Initialize our number of taps (windows size)
   // Let's stick with odd numbers only because math is easier
   _numTaps = (numTaps % 2 == 0) ? numTaps + 1 : numTaps;
   
   // Set our minimum number of samples.
   _minNumSamples = 5;
   
   // We have no timestamp yet, mark accordingly
   _firstSampleTimestamp = INT_MIN;
   
   // Allocate an internal buffer and taps buffer (because we have to?),
   // then compute LPF coefficients
   int bufSize = 0;
   int specSize = 0;
   check_sts( ippsFIRGenGetBufferSize( numTaps, &bufSize ) )
   _ippInternalBuffer1 = ippsMalloc_8u( bufSize );
   Ipp64f * taps = ippsMalloc_64f( numTaps * sizeof(Ipp64f) );
   check_sts( ippsFIRGenLowpass_64f( normalizedFrequency,
      taps,
      numTaps,
      ippWinBartlett,
      ippTrue,
      _ippInternalBuffer1
      ) )

   // Initialize the internal FIR structure
   check_sts( ippsFIRSRGetSize( numTaps,
      ipp64f,
      &specSize,
      &bufSize ) )
   _internalFirStructureIPP = ippsMalloc_8u( specSize );
   check_sts( ippsFIRSRInit_64f( taps,
      numTaps,
      ippAlgDirect,
      (IppsFIRSpec_64f*)_internalFirStructureIPP ) )
      
   // Create another internal buffer because I guess we need this when filtering
   _ippInternalBuffer2 = ippsMalloc_8u( bufSize );
   
   // Cleanup
   ippsFree( taps );
}
void Smooth_lpfIpp::Uninitialize()
{
   // Cleanup
   if ( _ippInternalBuffer1 )
      ippsFree( _ippInternalBuffer1 );
   if ( _ippInternalBuffer2 )
      ippsFree( _ippInternalBuffer2 );
   if ( _internalFirStructureIPP )
      ippsFree( _internalFirStructureIPP );
   
   _ippInternalBuffer1 = nullptr;
   _ippInternalBuffer2 = nullptr;
   _internalFirStructureIPP = nullptr;
}
void Smooth_lpfIpp::AddSample( int sampleTimestamp, double value )
{
   // If this is the first sample ever, set it
   if ( INT_MIN == _firstSampleTimestamp )
   {
      _firstSampleTimestamp = sampleTimestamp;
   }
      
   // If we have old samples and this sample is beyond the end of our delay line
   int insertionIndex = sampleTimestamp - _firstSampleTimestamp;
   if ( _newSamples.size() )
      while ( insertionIndex > (int)_newSamples.size() )
         // Go DC (flatline)
         _newSamples.push_back( _newSamples.back() );
      
   // As long as this simple isn't back in time (in which case don't add it)
   if ( insertionIndex >= (int)_newSamples.size() )
      _newSamples.push_back( value );
}
void Smooth_lpfIpp::AddSamples( int firstSampleTimestamp,
   const std::vector< double > & samples )
{
   // If this is the first sample ever, set it
   if ( INT_MIN == _firstSampleTimestamp )
      _firstSampleTimestamp = firstSampleTimestamp;
      
   // TODO: Implement as needed
   (void)samples;
   throw std::runtime_error( "Not implemented" );
}
int Smooth_lpfIpp::Apply( std::vector< double > & ref_smoothedSamples,
   bool flush )
{
   size_t numSamples = _newSamples.size();
   
   // If we don't have enough data
   int totalNumSamples = int(_delayLine.size() + numSamples);
   if ( totalNumSamples < _minNumSamples )
   {
      // Just copy over
      ref_smoothedSamples.resize( numSamples );
      memcpy( &ref_smoothedSamples[0],
         &_newSamples[0],
         numSamples * sizeof( double) );
      
      // Move our window, making sure to keep enough data for the next call.
      // This uses the last few filtered samples and then the remaining unfiltered samples
      int returnValue = _firstSampleTimestamp;
      _firstSampleTimestamp += int(numSamples);
      _delayLine = _newSamples;
      _newSamples.clear();
      return returnValue;
   }
   
   // Handle flushing frames by duplicating the last input value over and over,
   // creating a DC rolloff at the end
   int numExtraFlushSamples = 0;
   while ( flush &&
      numExtraFlushSamples < GetSampleShift() )
   {
      _newSamples.push_back( _newSamples.back() );
      ++numExtraFlushSamples;
   }
   numSamples += numExtraFlushSamples;

   // Return the first timestamp
   int returnValue = _firstSampleTimestamp - GetSampleShift();
   
   // If we don't have a delay line (first time)
   double * filteredResult = nullptr;
   if ( !_delayLine.size() )
   {
      // We need to pre-roll to account for the shift caused by the filter.
      // We need at least (numTaps - 1) elements as defined by IPP:
      //    https://software.intel.com/en-us/ipp-dev-reference-firsr
      // Pre-roll with a DC value that is the same as the first sample
      _delayLine = std::vector< double >( _numTaps - 1, _newSamples[0] );
      
      // We will skip the first few results because they will be from this artificial delay line.
      // So we need an intermediate output as well as the original timestamp back
      filteredResult = new double[ numSamples ];
      returnValue = _firstSampleTimestamp;
   }
   else
   {
      // Allocate memory for the filtered results and get a pointer to it
      ref_smoothedSamples.resize( numSamples );
      filteredResult = &ref_smoothedSamples[0];
   }
   
   // Perform the filtering
   check_sts( ippsFIRSR_64f( &_newSamples[0],
      filteredResult,
      numSamples,
      (IppsFIRSpec_64f*)_internalFirStructureIPP,
      &_delayLine[0],
      NULL,
      _ippInternalBuffer2) )
      
   // If this was the first time
   if ( ref_smoothedSamples.size() == 0 ||
      filteredResult != &ref_smoothedSamples[0] )
   {
      // Adjust, allocate output, copy the data from our temp buffer.
      // We need to skip the first delayed output samples since they are from the initial delay line.
      numSamples -= GetSampleShift();
      ref_smoothedSamples.resize( numSamples );
      memcpy( &ref_smoothedSamples[0],
         (filteredResult + GetSampleShift()),
         numSamples * sizeof(double) );
         
      delete[] filteredResult;
      filteredResult = &ref_smoothedSamples[0];
   }
      
   // Move our window, making sure to keep enough data for the next call.
   // This uses the last few filtered samples and then the remaining unfiltered samples
   _firstSampleTimestamp += numSamples;
   _delayLine.clear();
   for ( int i = 0; i < GetSampleShift(); ++i )
      _delayLine.push_back( filteredResult[ numSamples - GetSampleShift() + i ] );
   _delayLine.insert( std::begin(_delayLine) + GetSampleShift(), std::begin(_newSamples) + (numSamples - GetSampleShift()), std::end(_newSamples) );
   _newSamples.clear();

   return returnValue;
}
