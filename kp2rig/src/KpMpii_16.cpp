#define _USE_MATH_DEFINES
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "KpMpii_16.hpp"
#include "KpToRigHelper.hpp"
#include "RigToKpHelper.hpp"
#include "KpHelper.hpp"
#include "Utility.hpp"

KpMpii_16::KpMpii_16( std::string kpType,
   const std::map< KEYPOINT_TYPE, int > & kpLayout )
   : Pose( kpType, kpLayout )
{
}
KpMpii_16::KpMpii_16( const KpMpii_16 & rhs )
   : Pose( rhs )
{
   _keypoints = rhs._keypoints;
   _rigPose = rhs._rigPose;
   _rigCreated = rhs._rigCreated;
   _coordinateSystem = rhs._coordinateSystem;
}
RigPose & KpMpii_16::GenerateRig()
{
   if ( _rigCreated )
      return _rigPose;
   
   _rigPose.Timestamp( Timestamp() );
   
   // Spine, including baseNeck and baseHead
   KpToRigHelper::HandleSpine( *this );
   
   // Legs (not feet)
   KpToRigHelper::HandleLegs( *this );

   // Arms (not hands)
   KpToRigHelper::HandleArms( *this );
   
   // Deal with hands and feet
   HandleHands();
   HandleFeet();
   
   _rigPose.KpType( KpType() );
   _rigCreated = true;
   return _rigPose;
}
void KpMpii_16::FromRigPose( const class RigPose & rigPose )
{
   _rigPose = rigPose;
   
   RigToKpHelper::HandleSpine( *this );
   RigToKpHelper::HandleLegs( *this );
   RigToKpHelper::HandleArms( *this );
   
   _timestamp = rigPose.Timestamp();
   _rigCreated = true;
}
void KpMpii_16::InputDataToArray( std::vector< double > & serializedList )
{
   for ( size_t i = 0; i < InputDataSize(); ++i )
      serializedList.push_back( *((double *)&_keypoints + i) );
}
bool KpMpii_16::ValidateRig() const
{
   try
   {
      if ( !KpHelper::ValidateSpine( *this ) )
         return false;
      if ( !KpHelper::ValidateLegs( *this ) )
         return false;
      if ( !KpHelper::ValidateArms( *this ) )
         return false;
   }
   catch ( std::out_of_range & )
   {
      return false;
   }
      
   return true;
}
void KpMpii_16::CoordinateSystem( std::array< double, 3 > value )
{
   _coordinateSystem = value;
   std::array< double, 3 > * keypointsByOffset = reinterpret_cast< std::array< double, 3 > * >( &_keypoints );
   
   for ( size_t i = 0; i < InputDataSize() / 3; ++i )
   {
      keypointsByOffset->at( 0 ) *= _coordinateSystem[ 0 ];
      keypointsByOffset->at( 1 ) *= _coordinateSystem[ 1 ];
      keypointsByOffset->at( 2 ) *= _coordinateSystem[ 2 ];
      ++keypointsByOffset;
   }

   if ( _coordinateSystem[ 2 ] < 0 )
   {
      KpHelper::KeypointsRhToLh( *this );
   }
}
void KpMpii_16::HandleHands()
{
   Rig & rig = _rigPose.GetRig();
   Rig restPose = Rig::RestPoseHumanoid();
   
   // Guess the hands, starting with the wrist
   rig.rWrist.length = (rig.rElbow.length / restPose.rElbow.length) * restPose.rWrist.length;
   rig.rWrist.quaternion = { 0, 0, 0, 1 };
   rig.rWrist.quaternionAbs = rig.rElbow.quaternionAbs;
   rig.lWrist.length = (rig.lElbow.length / restPose.lElbow.length) * restPose.lWrist.length;
   rig.lWrist.quaternion = { 0, 0, 0, 1 };
   rig.lWrist.quaternionAbs = rig.lElbow.quaternionAbs;
}
void KpMpii_16::HandleFeet()
{
   Rig & rig = _rigPose.GetRig();
   Rig restPose = Rig::RestPoseHumanoid();
  
   // Guess the ankle
   rig.rAnkle.length = (rig.rKnee.length / restPose.rKnee.length) * restPose.rAnkle.length;
   rig.rAnkle.quaternion = {0,0,0,1};
   rig.rAnkle.quaternionAbs = Utility::QuaternionToRaw( Utility::RawToQuaternion( rig.rKnee.quaternionAbs ) * Utility::RawToQuaternion( restPose.rAnkle.quaternion ) );
   rig.lAnkle.length = (rig.lKnee.length / restPose.lKnee.length) * restPose.lAnkle.length;
   rig.lAnkle.quaternion = {0,0,0,1};
   rig.lAnkle.quaternionAbs = Utility::QuaternionToRaw( Utility::RawToQuaternion( rig.lKnee.quaternionAbs ) * Utility::RawToQuaternion( restPose.lAnkle.quaternion ) );
   
   // Guess the toes
   rig.rToeBase.length = (rig.rAnkle.length / restPose.rAnkle.length) * restPose.rToeBase.length;
   rig.rToeBase.quaternion = {0,0,0,1};
   rig.rToeBase.quaternionAbs = Utility::QuaternionToRaw( Utility::RawToQuaternion( rig.rAnkle.quaternionAbs ) * Utility::RawToQuaternion( restPose.rToeBase.quaternion ) );
   rig.lToeBase.length = (rig.lAnkle.length / restPose.lAnkle.length) * restPose.lToeBase.length;
   rig.lToeBase.quaternion = {0,0,0,1};
   rig.lToeBase.quaternionAbs = Utility::QuaternionToRaw( Utility::RawToQuaternion( rig.lAnkle.quaternionAbs ) * Utility::RawToQuaternion( restPose.lToeBase.quaternion ) );
}
