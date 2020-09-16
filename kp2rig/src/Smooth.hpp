#ifndef Smooth_hpp
#define Smooth_hpp

#include <stdio.h>
#include <vector>

// Interface for smoothing noisy data in a floating-point array. Each sample in the array
// is assumed to be temporally contiguous.
//
// The term "smooth" is used because it's more jazzy than "filtering",
// and because it better describes the resulting effect for the specific use case of
// smoothing noisy joint data. Because joints are defined by multiple values that can be smoothed
// independently, you can achieve joint filtering by instantiating three of these Smooth objects: one
// each for X, Y, and Z values.
//
// IMPLEMENTING A CUSTOM SMOOTH FILTER
// -----------------------------------
// 1. Create new class 'Smooth_<your filter type>' in new files named 'Smooth_<your filter type>'.hpp
//    and 'Smooth_<your filter type>'.cpp.
//    Ensure these names begin with "Smooth_" for correctness.
// 2. Your new class will inherit from the 'Smooth' interface below and implement all virtual functions.
//    This is a pure interface with no implemention proivded, so you're on your own.
// 3. Add a new entry in the SMOOTH_TYPE enumeration in @SmoothFactory.hpp.
// 4. #include your class at the top of @SmoothFactory.cpp (not @SmoothFactory.hpp).
// 5. Implement the creation logic for the following functions:
//    - SmoothFactory::SmoothType()
//    - SmoothFactory::Create()
// 6. Add your new files to the CMakeLists.txt so they will be built.
// 7. Add any 3rd-party dependencies to the CMakeLists.txt.
class Smooth
{
public:   
   virtual ~Smooth(){}
   
   // @numTaps is the window size
   // @normalizedFrequency is in units of cycles/sample, or 1/numSamplesInCuttoffFreq
   virtual void Initialize( int numTaps,
      double normalizedFrequency ) = 0;
      
   // Perform any tear-down here to "unwind" what was setup in the call to Initialize().
   // Initialize()/Uninitialize are allowed to be called multiple times.
   virtual void Uninitialize() = 0;
   
   // Add the next sample
   virtual void AddSample( int sampleTimestamp, double value ) = 0;
   
   // Add the next chunk of temporally-contiguous samples
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
