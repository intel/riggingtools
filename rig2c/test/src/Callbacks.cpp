#include <utility>
#include "Callbacks.hpp"
#include "Utility.hpp"
#include "rig2cDelegates.h"

std::string Callbacks::errorString = "";
int Callbacks::numBoundCallbacks = 0;
int Callbacks::numFrameCallbacks = 0;

void Callbacks::OnError( const char * rigId,
   rig_RETURN_CODE error,
   const char * description )
{
   printf( "FAIL %d: %s\n", error, description );
   (void)rigId;
   (void)error;
   (void)description;
}
void Callbacks::OnBounds( const char * rigId,
   int startTimestamp,
   int endTimestamp )
{
   ++numBoundCallbacks;
   
   // Get the name and type
   char type[512], name[512];
   (rig_getRigInfoDelegate(Utility::GetInstance()->GetFunctions()[ "rig_getRigInfo" ]))( "", rigId, "type", type, sizeof(type) );
   (rig_getRigInfoDelegate(Utility::GetInstance()->GetFunctions()[ "rig_getRigInfo" ]))( "", rigId, "name", name, sizeof(name) );

   (void)startTimestamp;
   (void)endTimestamp;
}
void Callbacks::OnFrame( const char * rigId,
    int frameTimestamp,
    const double * locationXYZ,
    const double * boneRotations,
    int numBoneRotations,
    const double * boneLengths,
    int numBoneLengths,
    const double * boneOffsets,
    int numBoneOffsets )
{
   ++numFrameCallbacks;
   
   // Test the arrays by accessing every element
   for ( int i = 0; i < 3; ++i )
   {
      double value = locationXYZ[i];
      if ( std::abs(value) > 1000000 )
      {
         printf( "'%s' frame %d, location%c: Unreasonable value %.2f\n",
            rigId,
            frameTimestamp,
            (char)(i+int('X')),
            value );
         errorString = "At least one unreasonable value";
         break;
      }
   }
   for ( int i = 0; i < numBoneRotations; ++i )
   {
      for ( int j = 0; j < 4; ++j )
      {
         double value = boneRotations[i*4+j];
         if ( std::abs(value) > 1 )
         {
            printf( "'%s' frame %d, rotation[%d].%c: Unreasonable value %.10f\n",
               rigId,
               frameTimestamp,
               i,
               j==3?'w':(char)(j+int('x')),
               value );
            errorString = "At least one unreasonable value";
            break;
         }
      }
   }
   for ( int i = 0; i < numBoneLengths; ++i )
   {
      double value = boneLengths[i];
      if ( value > 1000 || value <= 0.01 )
      {
         printf( "'%s' frame %d, length[%d]: Unreasonable value %.2f\n",
            rigId,
            frameTimestamp,
            i,
            value );
         errorString = "At least one unreasonable value";
         break;
      }
   }
   for ( int i = 0; i < numBoneOffsets; ++i )
   {
      for ( int j = 0; j < 3; ++j )
      {
         double value = boneOffsets[i*3+j];
         if ( std::abs(value) > 100 )
         {
            printf( "'%s' frame %d, offset[%d].%c: Unreasonable value %.2f\n",
               rigId,
               frameTimestamp,
               i,
               (char)(i+int('X')),
               value );
            errorString = "At least one unreasonable value";
            break;
         }
      }
   }
}
