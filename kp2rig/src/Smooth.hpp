#ifndef Smooth_hpp
#define Smooth_hpp

#include <stdio.h>
#include <vector>

// Interface for smoothing noisy data from a single joint
class Smooth
{
public:
   enum SMOOTH_TYPE
   {
      SMOOTH_TYPE_NONE,
      SMOOTH_TYPE_LPF_IPP,
      SMOOTH_TYPE_UNKNOWN
   };
   
   virtual ~Smooth(){}
   
   // @numTaps is the window size
   // @normalizedFrequency is in units of cycles/sample, or 1/numSamplesInCuttoffFreq
   virtual void Initialize( int numTaps,
      double normalizedFrequency ) = 0;
      
   virtual void Uninitialize() = 0;
   virtual void AddSample( int sampleTimestamp, double value ) = 0;
   virtual void AddSamples( int firstSampleTimestamp,
      const std::vector< double > & samples ) = 0;
   
   // @return the number of samples delayed by this filter
   virtual int GetSampleShift() const = 0;
      
   // @ref_smoothedSamples is updated with the filtered samples, starting at the returned timestamp
   // @flush Set to true at the end to get the last few filtered samples
   // @return the timestamp of the first sample, shifted as necessary
   virtual int Apply( std::vector< double > & ref_smoothedSamples,
      bool flush = false ) = 0;
};
#endif
