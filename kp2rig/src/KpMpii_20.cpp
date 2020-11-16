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
   
   // left foot tip
   Eigen::Vector3d parentLocation = Utility::RawToVector(_keypoints.leftAnkle);
   double boneLength = rig.lAnkle.length / ankleToeRatio;
   Eigen::Vector3d boneVector = Utility::RawToQuaternion( rig.lAnkle.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   Eigen::Vector3d childLocation = parentLocation + (boneVector*boneLength);
   _keypoints.leftFootTip = Utility::VectorToRaw( childLocation );
   
   // right foot tip
   parentLocation = Utility::RawToVector(_keypoints.rightAnkle);
   boneLength = rig.rAnkle.length / ankleToeRatio;
   boneVector = Utility::RawToQuaternion( rig.rAnkle.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   _keypoints.rightFootTip = Utility::VectorToRaw( childLocation );
   
   // Handle supplimentary joints
   for ( auto jointPair : rigPose.SupplimentaryJoints )
   {
      // Get the keypoint type
      auto keypointType = StrToKpType( jointPair.second.parentName );
      
      // Get the parent location
      // I'm using both the keypoint and the rig to avoid walking the entire hiearchy
      const Joint & parentJoint = rigPose.GetRig().GetJoint( jointPair.second.parentName );
      Eigen::Vector3d boneVector = Utility::RawToQuaternion( parentJoint.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() ) * parentJoint.length;
      auto parentLocation = Utility::RawToVector( Keypoint( keypointType ) ) + boneVector;
      
      // Get the bone vector
      boneVector = Utility::RawToQuaternion( jointPair.second.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() ) * jointPair.second.length;

      // Add the keypoint
      Keypoint( Utility::VectorToRaw( parentLocation + boneVector ), keypointType );
   }
   
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
      if ( !ValidateFeet() )
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
      
      std::swap( _keypoints.rightFootTip, _keypoints.leftFootTip );
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
   // We have an ankle, heel, and tip of foot keypoint for each foot.
   // Our rig requires an ankle and ball of foot (centered), each with a rotation.
   //
   //    * ankle                     * ankle
   //                        -->           ball *   * tip
   //   * heel        * tip
   //-------------------------------------------------------------------
   Rig & rig = _rigPose.GetRig();
   Rig restPose = Rig::RestPoseHumanoid();
   
   Eigen::Vector3d rAnkle( _keypoints.rightAnkle[0], _keypoints.rightAnkle[1], _keypoints.rightAnkle[2] );
   Eigen::Vector3d lAnkle( _keypoints.leftAnkle[0], _keypoints.leftAnkle[1], _keypoints.leftAnkle[2] );
   Eigen::Vector3d rFootTip( _keypoints.rightFootTip[0], _keypoints.rightFootTip[1], _keypoints.rightFootTip[2] );
   Eigen::Vector3d lFootTip( _keypoints.leftFootTip[0], _keypoints.leftFootTip[1], _keypoints.leftFootTip[2] );
   Eigen::Vector3d rHeel( _keypoints.rightHeel[0], _keypoints.rightHeel[1], _keypoints.rightHeel[2] );
   Eigen::Vector3d lHeel( _keypoints.leftHeel[0], _keypoints.leftHeel[1], _keypoints.leftHeel[2] );
   Eigen::Vector3d rAnkleToTip = rFootTip - rAnkle;
   Eigen::Vector3d lAnkleToTip = lFootTip - lAnkle;
   //Eigen::Vector3d rHeelToTip = rFootTip - rHeel;
   //Eigen::Vector3d lHeelToTip = lFootTip - lHeel;
   
   // Assumptions:
   //  a) both feet are the same size
   //  b) the distance from the ankle to the ball of the foot is the same as from the heel to the ball of the foot
   // Armed with these somewhat sketchy assumptions, use the rest pose to determine the length ratio from ankle to ball/tip of the foot.
   const double ballTipLengthRatio = restPose.rAnkle.length / (restPose.rAnkle.length + restPose.rToeBase.length);

   // Absolute rotations of ankles if bottom of foot is perpendicular to straight legs (like in our rest pose)
   Eigen::Quaterniond rAnkleRestPoseAdjustment = Utility::RawToQuaternion( rig.rKnee.quaternionAbs ) * Utility::RawToQuaternion( restPose.rAnkle.quaternion );
   Eigen::Quaterniond lAnkleRestPoseAdjustment = Utility::RawToQuaternion( rig.lKnee.quaternionAbs ) * Utility::RawToQuaternion( restPose.lAnkle.quaternion );
   
   // Ankle
   rig.rAnkle.length = rAnkleToTip.norm() * ballTipLengthRatio;
   Eigen::Quaterniond q1 = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitY(), rAnkleRestPoseAdjustment.inverse()._transformVector( rAnkleToTip ) );
   rig.rAnkle.quaternion = Utility::QuaternionToRaw( q1 );
   rig.rAnkle.quaternionAbs = Utility::QuaternionToRaw( rAnkleRestPoseAdjustment * q1 );
   rig.lAnkle.length = lAnkleToTip.norm() * ballTipLengthRatio;
   q1 = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitY(), lAnkleRestPoseAdjustment.inverse()._transformVector( lAnkleToTip ) );
   rig.lAnkle.quaternion = Utility::QuaternionToRaw( q1 );
   rig.lAnkle.quaternionAbs = Utility::QuaternionToRaw( lAnkleRestPoseAdjustment * q1 );
   
   // Toe base (ball of foot)
   rig.rToeBase.length = rAnkleToTip.norm() * (1-ballTipLengthRatio);
   rig.rToeBase.quaternion = restPose.rToeBase.quaternion;
   rig.rToeBase.quaternionAbs = Utility::QuaternionToRaw( Utility::RawToQuaternion( rig.rAnkle.quaternionAbs ) * Utility::RawToQuaternion( restPose.rToeBase.quaternion ) );
   rig.lToeBase.length = lAnkleToTip.norm() * (1-ballTipLengthRatio);
   rig.lToeBase.quaternion = restPose.rToeBase.quaternion;
   rig.lToeBase.quaternionAbs = Utility::QuaternionToRaw( Utility::RawToQuaternion( rig.lAnkle.quaternionAbs ) * Utility::RawToQuaternion( restPose.lToeBase.quaternion ) );
   
   // Heel is a supplimentary joint
   _rigPose.SupplimentaryJoints[ "rightHeel" ] = SupplimentaryJoint( "rightHeel", "rightAnkle", KpToRigHelper::CreateJoint( rig.rAnkle, rHeel - rAnkle ) );
   _rigPose.SupplimentaryJoints[ "leftHeel"  ] = SupplimentaryJoint( "leftHeel",  "leftAnkle",  KpToRigHelper::CreateJoint( rig.lAnkle, lHeel - lAnkle ) );
}
bool KpMpii_20::ValidateFeet() const
{
   const Rig & rig = _rigPose.GetRig();
   Rig restPose = Rig::RestPoseHumanoid();
   constexpr double tolerance = 0.25;
   
   // Get the absolute location of the ankles
   auto rAnkleLocation = RigToKpHelper::WorldCoordinates( rig, Rig::RANKLE );
   auto lAnkleLocation = RigToKpHelper::WorldCoordinates( rig, Rig::LANKLE );

   // Heel
   auto joint = _rigPose.SupplimentaryJoints.at( "rightHeel" );
   Eigen::Quaterniond q = Utility::RawToQuaternion( joint.quaternionAbs );
   Eigen::Vector3d vec = q._transformVector( Eigen::Vector3d::UnitY() ) * joint.length;
   double distance = (Utility::RawToVector(rAnkleLocation) + vec - Utility::RawToVector(Keypoint(RIGHT_HEEL))).norm();
   if ( distance > tolerance )
      return false;
   joint = _rigPose.SupplimentaryJoints.at( "leftHeel" );
   q = Utility::RawToQuaternion( joint.quaternionAbs );
   vec = q._transformVector( Eigen::Vector3d::UnitY() ) * joint.length;
   distance = (Utility::RawToVector(lAnkleLocation) + vec - Utility::RawToVector(Keypoint(LEFT_HEEL))).norm();
   if ( distance > tolerance )
      return false;
      
   // Foot tip
   q = Utility::RawToQuaternion( rig.rAnkle.quaternionAbs );
   vec = q._transformVector( Eigen::Vector3d::UnitY() ) * (rig.rAnkle.length + rig.rToeBase.length);
   distance = (Utility::RawToVector(rAnkleLocation) + vec - Utility::RawToVector(Keypoint(RIGHT_FOOT_TIP))).norm();
   if ( distance > tolerance )
      return false;
   q = Utility::RawToQuaternion( rig.lAnkle.quaternionAbs );
   vec = q._transformVector( Eigen::Vector3d::UnitY() ) * (rig.lAnkle.length + rig.lToeBase.length);
   distance = (Utility::RawToVector(lAnkleLocation) + vec - Utility::RawToVector(Keypoint(LEFT_FOOT_TIP))).norm();
   if ( distance > tolerance )
      return false;
      
   return true;
}
