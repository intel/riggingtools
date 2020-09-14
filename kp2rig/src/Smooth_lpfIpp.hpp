#ifndef Smooth_lpfIpp_hpp
#define Smooth_lpfIpp_hpp

#ifdef HAVE_IPP

#include "Smooth.hpp"
asdasdf
class Smooth_lpfIpp : public Smooth
{
public:
   virtual ~Smooth_lpfIpp();
   Smooth_lpfIpp() = default;
   Smooth_lpfIpp( Smooth_lpfIpp && move );
   
   virtual void Initialize( int numTaps,
      double normalizedFrequency );
   virtual void Uninitialize();
   virtual void AddSample( int sampleTimestamp, double value );
   virtual void AddSamples( int firstSampleTimestamp,
      const std::vector< double > & samples );
   virtual int GetSampleShift() const;
   virtual int Apply( std::vector< double > & ref_smoothedSamples,
      bool flush = false );
      
private:
   std::vector< double > _delayLine;
   std::vector< double > _newSamples;
   int _firstSampleTimestamp;
   void * _internalFirStructureIPP = nullptr;
   unsigned char * _ippInternalBuffer1 = nullptr;
   unsigned char * _ippInternalBuffer2 = nullptr;
   int _numTaps;
   int _minNumSamples;
};

inline int Smooth_lpfIpp::GetSampleShift() const
{
   return (_numTaps - 1)/2;
}
#endif

#endif
