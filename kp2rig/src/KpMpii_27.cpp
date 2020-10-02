#define _USE_MATH_DEFINES
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "KpMpii_27.hpp"
#include "KpToRigHelper.hpp"
#include "RigToKpHelper.hpp"
#include "KpHelper.hpp"
#include "Utility.hpp"

// Helper functions
Eigen::Vector3d GetTipOfFoot( const Eigen::Vector3d & bigToe,
   const Eigen::Vector3d & smallToe,
   const Eigen::Vector3d & heel );
void GetToes( const RigPose & rigPose );
   
KpMpii_27::KpMpii_27( std::string kpType,
   const std::map< KEYPOINT_TYPE, int > & kpLayout )
   : Pose( kpType, kpLayout )
{
}
KpMpii_27::KpMpii_27( const KpMpii_27 & rhs )
   : Pose( rhs )
{
   _keypoints = rhs._keypoints;
   _rigPose = rhs._rigPose;
   _rigCreated = rhs._rigCreated;
   _coordinateSystem = rhs._coordinateSystem;
}
RigPose & KpMpii_27::GenerateRig()
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
void KpMpii_27::FromRigPose( const class RigPose & rigPose )
{
   _rigPose = rigPose;
   Rig & rig = _rigPose.GetRig();
   
   RigToKpHelper::HandleSpine( *this );
   RigToKpHelper::HandleLegs( *this );
   RigToKpHelper::HandleArms( *this );
   
   GetToes( rigPose );
   
   // TODO: Use the heels from the supplimentary joints
   Eigen::Vector3d parentLocation = Utility::RawToVector(_keypoints.leftAnkle);
   double boneLength = rig.lAnkle.length / 2;
   Eigen::Vector3d boneVector = Utility::RawToQuaternion( rig.lKnee.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   Eigen::Vector3d childLocation = parentLocation + (boneVector*boneLength);
   _keypoints.leftHeel = Utility::VectorToRaw( childLocation );
   
   parentLocation = Utility::RawToVector(_keypoints.rightAnkle);
   boneLength = rig.rAnkle.length / 2;
   boneVector = Utility::RawToQuaternion( rig.rKnee.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   _keypoints.rightHeel = Utility::VectorToRaw( childLocation );
   
   _timestamp = rigPose.Timestamp();
   _rigCreated = true;
}
void KpMpii_27::InputDataToArray( std::vector< double > & serializedList )
{
   for ( size_t i = 0; i < InputDataSize(); ++i )
      serializedList.push_back( *((double *)&_keypoints + i) );
}
bool KpMpii_27::ValidateRig() const
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
void KpMpii_27::CoordinateSystem( std::array< double, 3 > value )
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
      
      std::swap( _keypoints.rightBigToe, _keypoints.leftBigToe );
      std::swap( _keypoints.rightSmallToe, _keypoints.leftSmallToe );
      std::swap( _keypoints.rightHeel, _keypoints.leftHeel );
      std::swap( _keypoints.rightEye, _keypoints.leftEye );
      std::swap( _keypoints.rightEar, _keypoints.leftEar );
   }
}
void KpMpii_27::HandleHands()
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
void KpMpii_27::HandleFeet()
{
   Rig & rig = _rigPose.GetRig();
   Rig restPose = Rig::RestPoseHumanoid();
   
   Eigen::Vector3d forwardVector( 0, 0, 1 );
   Eigen::Vector3d rAnkle( _keypoints.rightAnkle[0], _keypoints.rightAnkle[1], _keypoints.rightAnkle[2] );
   Eigen::Vector3d lAnkle( _keypoints.leftAnkle[0], _keypoints.leftAnkle[1], _keypoints.leftAnkle[2] );
   Eigen::Vector3d rBigToe( _keypoints.rightBigToe[0], _keypoints.rightBigToe[1], _keypoints.rightBigToe[2] );
   Eigen::Vector3d rSmallToe( _keypoints.rightSmallToe[0], _keypoints.rightSmallToe[1], _keypoints.rightSmallToe[2] );
   Eigen::Vector3d rHeel( _keypoints.rightHeel[0], _keypoints.rightHeel[1], _keypoints.rightHeel[2] );
   Eigen::Vector3d lBigToe( _keypoints.leftBigToe[0], _keypoints.leftBigToe[1], _keypoints.leftBigToe[2] );
   Eigen::Vector3d lSmallToe( _keypoints.leftSmallToe[0], _keypoints.leftSmallToe[1], _keypoints.leftSmallToe[2] );
   Eigen::Vector3d lHeel( _keypoints.leftHeel[0], _keypoints.leftHeel[1], _keypoints.leftHeel[2] );
   Eigen::Quaterniond rKneeRotation = Utility::RawToQuaternion( rig.rKnee.quaternionAbs );
   Eigen::Quaterniond lKneeRotation = Utility::RawToQuaternion( rig.lKnee.quaternionAbs );
   
   // Create a point at the tip of the foot somewhere between the big and small toe.
   // Note the vector between big and small toe is not naturally perpendicular to center line along length of foot.
   Eigen::Vector3d rToe = GetTipOfFoot( rBigToe, rSmallToe, rHeel );
   Eigen::Vector3d lToe= GetTipOfFoot( lBigToe, lSmallToe, lHeel );   
   
   Eigen::Vector3d rAnkleToToe = rToe - rAnkle;
   Eigen::Vector3d lAnkleToToe = lToe - lAnkle;
   const double ankleToeRatio = restPose.rAnkle.length / (restPose.rAnkle.length + restPose.rToeBase.length);
   
   // Rest poses
   Eigen::Quaterniond rAnkleRestPoseAdjustment = rKneeRotation * Utility::RawToQuaternion( restPose.rAnkle.quaternion );
   Eigen::Quaterniond lAnkleRestPoseAdjustment = lKneeRotation * Utility::RawToQuaternion( restPose.lAnkle.quaternion );
   
   // Ankle
   rig.rAnkle.length = rAnkleToToe.norm() * ankleToeRatio;
   Eigen::Quaterniond q1 = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitY(), rAnkleRestPoseAdjustment.inverse()._transformVector( rAnkleToToe ) );
   rig.rAnkle.quaternion = Utility::QuaternionToRaw( q1 );
   rig.rAnkle.quaternionAbs = Utility::QuaternionToRaw( rAnkleRestPoseAdjustment * q1 );
   rig.lAnkle.length = lAnkleToToe.norm() * ankleToeRatio;
   q1 = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitY(), lAnkleRestPoseAdjustment.inverse()._transformVector( lAnkleToToe ) );
   rig.lAnkle.quaternion = Utility::QuaternionToRaw( q1 );
   rig.lAnkle.quaternionAbs = Utility::QuaternionToRaw( lAnkleRestPoseAdjustment * q1 );
   
   // Toe base (ball of foot)
   rig.rToeBase.length = rAnkleToToe.norm() * (1-ankleToeRatio);
   rig.rToeBase.quaternion = {0,0,0,1};
   rig.rToeBase.quaternionAbs = Utility::QuaternionToRaw( Utility::RawToQuaternion( rig.rAnkle.quaternionAbs ) * Utility::RawToQuaternion( restPose.rToeBase.quaternion ) );
   rig.lToeBase.length = lAnkleToToe.norm() * (1-ankleToeRatio);
   rig.lToeBase.quaternion = {0,0,0,1};
   rig.lToeBase.quaternionAbs = Utility::QuaternionToRaw( Utility::RawToQuaternion( rig.lAnkle.quaternionAbs ) * Utility::RawToQuaternion( restPose.lToeBase.quaternion ) );
   
   // Right heel
   Joint rHeelJoint;
   Eigen::Vector3d rHeelVector = rHeel - rAnkle;
   q1 = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitY(), rKneeRotation.inverse()._transformVector( rHeelVector ) );
   rHeelJoint.length = rHeelVector.norm();
   rHeelJoint.quaternion = Utility::QuaternionToRaw( q1 );
   rHeelJoint.quaternionAbs = Utility::QuaternionToRaw( rKneeRotation * q1 );
   _rigPose._supplimentaryJoints.push_back( std::make_pair( "rightKnee", rHeelJoint ) );
   
   // Right big toe
   Joint rBigToeJoint;
   Eigen::Vector3d rBigToeVector = rBigToe - rAnkle;
   q1 = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitY(), rKneeRotation.inverse()._transformVector( rBigToeVector ) );
   rBigToeJoint.length = rBigToeVector.norm();
   rBigToeJoint.quaternion = Utility::QuaternionToRaw( q1 );
   rBigToeJoint.quaternionAbs = Utility::QuaternionToRaw( rKneeRotation * q1 );
   _rigPose._supplimentaryJoints.push_back( std::make_pair( "rightKnee", rBigToeJoint ) );
   
   // Right small toe
   Joint rSmallToeJoint;
   Eigen::Vector3d rSmallToeVector = rSmallToe - rAnkle;
   q1 = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitY(), rKneeRotation.inverse()._transformVector( rSmallToeVector ) );
   rSmallToeJoint.length = rSmallToeVector.norm();
   rSmallToeJoint.quaternion = Utility::QuaternionToRaw( q1 );
   rSmallToeJoint.quaternionAbs = Utility::QuaternionToRaw( rKneeRotation * q1 );
   _rigPose._supplimentaryJoints.push_back( std::make_pair( "rightKnee", rSmallToeJoint ) );
   
   // Left heel
   Joint lHeelJoint;
   Eigen::Vector3d lHeelVector = lHeel - lAnkle;
   q1 = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitY(), lKneeRotation.inverse()._transformVector( lHeelVector ) );
   lHeelJoint.length = lHeelVector.norm();
   lHeelJoint.quaternion = Utility::QuaternionToRaw( q1 );
   lHeelJoint.quaternionAbs = Utility::QuaternionToRaw( lKneeRotation * q1);
   _rigPose._supplimentaryJoints.push_back( std::make_pair( "leftKnee", rHeelJoint ) );
   
   // Left big toe
   Joint lBigToeJoint;
   Eigen::Vector3d lBigToeVector = lBigToe - lAnkle;
   q1 = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitY(), lKneeRotation.inverse()._transformVector( lBigToeVector ) );
   lBigToeJoint.length = lBigToeVector.norm();
   lBigToeJoint.quaternion = Utility::QuaternionToRaw( q1 );
   lBigToeJoint.quaternionAbs = Utility::QuaternionToRaw( lKneeRotation * q1 );
   _rigPose._supplimentaryJoints.push_back( std::make_pair( "leftKnee", lBigToeJoint ) );
   
   // Left small toe
   Joint lSmallToeJoint;
   Eigen::Vector3d lSmallToeVector = lSmallToe - lAnkle;
   q1 = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitY(), lKneeRotation.inverse()._transformVector( lSmallToeVector ) );
   lSmallToeJoint.length = lSmallToeVector.norm();
   lSmallToeJoint.quaternion = Utility::QuaternionToRaw( q1 );
   lSmallToeJoint.quaternionAbs = Utility::QuaternionToRaw( lKneeRotation * q1 );
   _rigPose._supplimentaryJoints.push_back( std::make_pair( "rightKnee", lSmallToeJoint ) );
}
Eigen::Vector3d GetTipOfFoot( const Eigen::Vector3d & bigToe,
   const Eigen::Vector3d & smallToe,
   const Eigen::Vector3d & heel )
{
   // Estimate the tip of the foot
   //
   // LEFT FOOT example:
   //
   //                         *      <--point t, estimated tip of foot
   //   point p1,                 *  <--point b, center of big toe (not the tip)
   //   estimated  -->        *
   //   point along     *            <--point s, center of small toe (not the tip)
   //   center line of
   //   foot
   //
   //
   //
   //
   //                         *        <--point h, heel
   //
   
   // This represents the distance between the center of the big toe
   // (where the nail meets the skin) and the tip of the longest toe,
   // as a ratio of the entire length of the foot.
   constexpr double bigToeCenterToFootTipRatio = 0.05;
   
   // The longest portion of the foot isn't centered between the big toe
   // and small toe, it is usually biased towards the big toe.
   // This respresents the distance between the center of the big toe
   // and the line connecting the tip of the foot to the heel.
   constexpr double bigToeCenterToFootLongestLineRatio = 0.39;
   
   // Get vector bs
   Eigen::Vector3d bs = smallToe - bigToe;
   
   // - Find point p1 along vector bs using fixed ratio
   bs *= bigToeCenterToFootLongestLineRatio;
   Eigen::Vector3d p1 = bigToe + bs;
   
   // Get vector hb
   Eigen::Vector3d hb = bigToe - heel;
   
   // Get vector hp1
   Eigen::Vector3d hp1 = p1 - heel;
   
   // set ||hp1|| = ||hb||(1 + ratio)
   hp1 = hp1.normalized() * hb.norm() * (1 + bigToeCenterToFootTipRatio);
   
   // Return the tip of the foot
   return heel + hp1;
}
void GetToes( const RigPose & rigPose )
{
   (void)rigPose;
   
   //const Rig & restPose = Rig::RestPoseHumanoid();
   //const double ankleToeRatio = restPose.rAnkle.length / (restPose.rAnkle.length + restPose.rToeBase.length);
   
   // left big toe
//   Eigen::Vector3d parentLocation = Utility::RawToVector(_keypoints.leftAnkle);
//   double boneLength = rig.lAnkle.length / ankleToeRatio;
//   Eigen::Vector3d boneVector = Utility::RawToQuaternion( rig.lAnkle.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
//   Eigen::Vector3d childLocation = parentLocation + (boneVector*boneLength);
//   _keypoints.leftToe = Utility::VectorToRaw( childLocation );
   
//   // left small toe
//   double boneLength = rig.lAnkle.length / ankleToeRatio;
//   Eigen::Vector3d boneVector = Utility::RawToQuaternion( rig.lAnkle.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
//   Eigen::Vector3d childLocation = parentLocation + (boneVector*boneLength);
//   _keypoints.leftToe = Utility::VectorToRaw( childLocation );
   
   // right toe
//   parentLocation = Utility::RawToVector(_keypoints.rightAnkle);
//   boneLength = rig.rAnkle.length / ankleToeRatio;
//   boneVector = Utility::RawToQuaternion( rig.rAnkle.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
//   childLocation = parentLocation + (boneVector*boneLength);
//   _keypoints.rightToe = Utility::VectorToRaw( childLocation );
}
