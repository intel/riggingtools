#define _USE_MATH_DEFINES
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "KpMop_19.hpp"
#include "KpToRigHelper.hpp"
#include "RigToKpHelper.hpp"
#include "KpHelper.hpp"
#include "Utility.hpp"

KpMop_19::KpMop_19( std::string kpType,
   const std::map< KEYPOINT_TYPE, int > & kpLayout )
   : Pose( kpType, kpLayout )
{
}
KpMop_19::KpMop_19( const KpMop_19 & rhs )
   : Pose( rhs )
{
   _keypoints = rhs._keypoints;
   _rigPose = rhs._rigPose;
   _rigCreated = rhs._rigCreated;
   _coordinateSystem = rhs._coordinateSystem;
}
RigPose & KpMop_19::GenerateRig()
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
void KpMop_19::FromRigPose( const class RigPose & rigPose )
{
   _rigPose = rigPose;
   
   RigToKpHelper::HandleSpine( *this );
   RigToKpHelper::HandleLegs( *this );
   RigToKpHelper::HandleArms( *this );
   
   _timestamp = rigPose.Timestamp();
   _rigCreated = true;
}
void KpMop_19::InputDataToArray( std::vector< double > & serializedList )
{
   for ( size_t i = 0; i < InputDataSize(); ++i )
      serializedList.push_back( *((double *)&_keypoints + i) );
}
bool KpMop_19::ValidateRig() const
{
   if ( !KpHelper::ValidateSpine( *this ) )
      return false;
   if ( !KpHelper::ValidateLegs( *this ) )
      return false;
   if ( !KpHelper::ValidateArms( *this ) )
      return false;
      
   return true;
}
void KpMop_19::CoordinateSystem( std::array< double, 3 > value )
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
void KpMop_19::HandleHands()
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
void KpMop_19::HandleFeet()
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
void KpMop_19::Keypoint( std::array< double, 3 > value, KEYPOINT_TYPE type )
{
   switch ( type )
   {
      case PELVIS: break;
      case BASE_HEAD: break;
      default: Keypoint( value, _kpLayout.at( type ) ); break;
   }
}
const std::array< double, 3 > & KpMop_19::Keypoint( KEYPOINT_TYPE type ) const
{
   try
   {
      return *((std::array< double, 3 > *)&_keypoints + _kpLayout.at( type ) );
   }
   catch ( std::out_of_range & )
   {
      std::stringstream ss;
      ss << "No keypoint found for '" << KpTypeToStr( type ) << "'. Check kpDescriptor.json";
      throw std::runtime_error( ss.str() );
   }
}
