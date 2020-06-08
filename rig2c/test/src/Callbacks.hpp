//
//  Callbacks.hpp
//  rig2cTest
//
//  Created by John A Harrison on 4/6/20.
//

#ifndef Callbacks_hpp
#define Callbacks_hpp

#include <stdio.h>
#include <string>
#include "rig2c.h"

class Callbacks
{
public:
   static int numFrameCallbacks;
   static int numBoundCallbacks;
   static std::string errorString;
   
   static void OnError( const char * rigId,
      rig_RETURN_CODE error,
      const char * description );
   static void OnBounds( const char * rigId,
      int startTimestamp,
      int endTimestamp );
   static void OnFrame( const char * rigId,
       int frameTimestamp,
       const double * locationXYZ,
       const double * boneRotations,
       int numBoneRotations,
       const double * boneLengths,
       int numBoneLengths,
       const double * boneOffsets,
       int numBoneOffsets );
};

#endif /* Callbacks_hpp */
