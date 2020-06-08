#define _USE_MATH_DEFINES
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "KpMpii_20.hpp"
#include "KpToRigHelper.hpp"
#include "RigToKpHelper.hpp"
#include "KpHelper.hpp"
#include "Utility.hpp"

KpMpii_20::KpMpii_20( std::string kpType,
   const std::map< KEYPOINT_TYPE, int > & kpLayout )
   : Pose( kpType, kpLayout )
{
}
KpMpii_20::KpMpii_20( const KpMpii_20 & rhs )
   : Pose( rhs )
{
   _keypoints = rhs._keypoints;
   _rigPose = rhs._rigPose;
   _rigCreated = rhs._rigCreated;
   _coordinateSystem = rhs._coordinateSystem;
}
RigPose & KpMpii_20::GenerateRig()
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
void KpMpii_20::FromRigPose( const class RigPose & rigPose )
{
   _rigPose = rigPose;
   Rig & rig = _rigPose.GetRig();
   const Rig & restPose = Rig::RestPoseHumanoid();
   const double ankleToeRatio = restPose.rAnkle.length / (restPose.rAnkle.length + restPose.rToeBase.length);
   
   RigToKpHelper::HandleSpine( *this );
   RigToKpHelper::HandleLegs( *this );
   RigToKpHelper::HandleArms( *this );
   
   // left toe
   Eigen::Vector3d parentLocation = Utility::RawToVector(_keypoints.leftAnkle);
   double boneLength = rig.lAnkle.length / ankleToeRatio;
   Eigen::Vector3d boneVector = Utility::RawToQuaternion( rig.lAnkle.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   Eigen::Vector3d childLocation = parentLocation + (boneVector*boneLength);
   _keypoints.leftToe = Utility::VectorToRaw( childLocation );
   
   // right toe
   parentLocation = Utility::RawToVector(_keypoints.rightAnkle);
   boneLength = rig.rAnkle.length / ankleToeRatio;
   boneVector = Utility::RawToQuaternion( rig.rAnkle.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   _keypoints.rightToe = Utility::VectorToRaw( childLocation );
   
   // Nov 2019, JH: I don't care about the heels since they aren't used, just trying to avoid validation errors here
   parentLocation = Utility::RawToVector(_keypoints.leftAnkle);
   boneLength = rig.lAnkle.length / 2;
   boneVector = Utility::RawToQuaternion( rig.lKnee.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   _keypoints.leftHeel = Utility::VectorToRaw( childLocation );
   
   parentLocation = Utility::RawToVector(_keypoints.rightAnkle);
   boneLength = rig.rAnkle.length / 2;
   boneVector = Utility::RawToQuaternion( rig.rKnee.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   _keypoints.rightHeel = Utility::VectorToRaw( childLocation );
   
   _timestamp = rigPose.Timestamp();
   _rigCreated = true;
}
void KpMpii_20::InputDataToArray( std::vector< double > & serializedList )
{
   for ( size_t i = 0; i < InputDataSize(); ++i )
      serializedList.push_back( *((double *)&_keypoints + i) );
}
bool KpMpii_20::ValidateRig() const
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
void KpMpii_20::CoordinateSystem( std::array< double, 3 > value )
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
      
      std::swap( _keypoints.rightToe, _keypoints.leftToe );
      std::swap( _keypoints.rightHeel, _keypoints.leftHeel );
   }
}
void KpMpii_20::HandleHands()
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
void KpMpii_20::HandleFeet()
{
   Rig & rig = _rigPose.GetRig();
   Rig restPose = Rig::RestPoseHumanoid();
   
   Eigen::Vector3d upVector( 0, 1, 0 );
   Eigen::Vector3d forwardVector( 0, 0, 1 );
   Eigen::Vector3d rAnkle( _keypoints.rightAnkle[0], _keypoints.rightAnkle[1], _keypoints.rightAnkle[2] );
   Eigen::Vector3d lAnkle( _keypoints.leftAnkle[0], _keypoints.leftAnkle[1], _keypoints.leftAnkle[2] );
   Eigen::Vector3d rToe( _keypoints.rightToe[0], _keypoints.rightToe[1], _keypoints.rightToe[2] );
   Eigen::Vector3d lToe( _keypoints.leftToe[0], _keypoints.leftToe[1], _keypoints.leftToe[2] );
   Eigen::Vector3d rAnkleToToe = rToe - rAnkle;
   Eigen::Vector3d lAnkleToToe = lToe - lAnkle;
   const double ankleToeRatio = restPose.rAnkle.length / (restPose.rAnkle.length + restPose.rToeBase.length);
   
   // Rest poses
   Eigen::Quaterniond rAnkleRestPoseAdjustment = Utility::RawToQuaternion( rig.rKnee.quaternionAbs ) * Utility::RawToQuaternion( restPose.rAnkle.quaternion );
   Eigen::Quaterniond lAnkleRestPoseAdjustment = Utility::RawToQuaternion( rig.lKnee.quaternionAbs ) * Utility::RawToQuaternion( restPose.lAnkle.quaternion );
   
   // Ankle
   rig.rAnkle.length = rAnkleToToe.norm() * ankleToeRatio;
   Eigen::Quaterniond q1 = Eigen::Quaterniond::FromTwoVectors( upVector, rAnkleRestPoseAdjustment.inverse()._transformVector( rAnkleToToe ) );
   rig.rAnkle.quaternion = Utility::QuaternionToRaw( q1 );
   rig.rAnkle.quaternionAbs = Utility::QuaternionToRaw( rAnkleRestPoseAdjustment * q1 );
   rig.lAnkle.length = lAnkleToToe.norm() * ankleToeRatio;
   q1 = Eigen::Quaterniond::FromTwoVectors( upVector, lAnkleRestPoseAdjustment.inverse()._transformVector( lAnkleToToe ) );
   rig.lAnkle.quaternion = Utility::QuaternionToRaw( q1 );
   rig.lAnkle.quaternionAbs = Utility::QuaternionToRaw( lAnkleRestPoseAdjustment * q1 );
   
   // Toe base (ball of foot)
   rig.rToeBase.length = rAnkleToToe.norm() * (1-ankleToeRatio);
   rig.rToeBase.quaternion = {0,0,0,1};
   rig.rToeBase.quaternionAbs = Utility::QuaternionToRaw( Utility::RawToQuaternion( rig.rAnkle.quaternionAbs ) * Utility::RawToQuaternion( restPose.rToeBase.quaternion ) );
   rig.lToeBase.length = lAnkleToToe.norm() * (1-ankleToeRatio);
   rig.lToeBase.quaternion = {0,0,0,1};
   rig.lToeBase.quaternionAbs = Utility::QuaternionToRaw( Utility::RawToQuaternion( rig.lAnkle.quaternionAbs ) * Utility::RawToQuaternion( restPose.lToeBase.quaternion ) );
}
