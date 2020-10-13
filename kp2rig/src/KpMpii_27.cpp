#define _USE_MATH_DEFINES
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "KpMpii_27.hpp"
#include "KpToRigHelper.hpp"
#include "RigToKpHelper.hpp"
#include "KpHelper.hpp"

// Helper functions
Eigen::Vector3d GetTipOfFoot( const Eigen::Vector3d & bigToe,
   const Eigen::Vector3d & smallToe,
   const Eigen::Vector3d & heel );
   
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
   
   RigToKpHelper::HandleSpine( *this );
   RigToKpHelper::HandleLegs( *this );
   RigToKpHelper::HandleArms( *this );
   
   // Handle supplimentary joints
   for ( auto joint : rigPose.SupplimentaryJoints )
   {
      // Get the keypoint type
      auto keypointType = StrToKpType( joint.parentName );
      
      // Get the parent location
      // I'm using both the keypoint and the rig to avoid walking the entire hiearchy
      const Joint & parentJoint = rigPose.GetRig().GetJoint( joint.parentName );
      Eigen::Vector3d boneVector = Utility::RawToQuaternion( parentJoint.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() ) * parentJoint.length;
      auto parentLocation = Utility::RawToVector( Keypoint( keypointType ) ) + boneVector;
      
      // Get the bone vector
      boneVector = Utility::RawToQuaternion( joint.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() ) * joint.length;

      // Add the keypoint
      Keypoint( Utility::VectorToRaw( parentLocation + boneVector ), keypointType );
   }
   
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
      if ( !ValidateFeet() )
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
   Eigen::Quaterniond rKneeRotationAbs = Utility::RawToQuaternion( rig.rKnee.quaternionAbs );
   Eigen::Quaterniond lKneeRotationAbs = Utility::RawToQuaternion( rig.lKnee.quaternionAbs );
   
   // Create a point at the tip of the foot somewhere between the big and small toe.
   // Note the vector between big and small toe is not naturally perpendicular to center line along length of foot.
   Eigen::Vector3d rToe = GetTipOfFoot( rBigToe, rSmallToe, rHeel );
   Eigen::Vector3d lToe= GetTipOfFoot( lBigToe, lSmallToe, lHeel );   
   
   Eigen::Vector3d rAnkleToToe = rToe - rAnkle;
   Eigen::Vector3d lAnkleToToe = lToe - lAnkle;
   const double ankleToeRatio = restPose.rAnkle.length / (restPose.rAnkle.length + restPose.rToeBase.length);
   
   // Rest poses
   Eigen::Quaterniond rAnkleRestPoseAdjustment = rKneeRotationAbs * Utility::RawToQuaternion( restPose.rAnkle.quaternion );
   Eigen::Quaterniond lAnkleRestPoseAdjustment = lKneeRotationAbs * Utility::RawToQuaternion( restPose.lAnkle.quaternion );
   
   // Ankle Rotations
   Eigen::Quaterniond rAnkleRotation = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitY(), rAnkleRestPoseAdjustment.inverse()._transformVector( rAnkleToToe.normalized() ) );
   Eigen::Quaterniond lAnkleRotation = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitY(), lAnkleRestPoseAdjustment.inverse()._transformVector( lAnkleToToe.normalized() ) );
   
   // Ankle roll
   // We don't have enough information to know the difference between toe roll and ankle roll,
   // so the same roll will be applied to both; this is good enough when stiff shoes are worn, but not ideal
   // when soft shoes are worn or bare foot.
   //   1 calculate the toes unit vector (small toe to big toe)
   //   2 adjust this toes vector so that it's perpendcular to the ankle vector
   //   3 apply the inverse ankle rotation to get the toes vector perpendicular to the up vector.
   //     The toes vector now lies along the xz plane
   //   4 determine the difference between the x vector (one positive, one negative) and the toes vector.
   //     Final rotation should be small unless an injury occurs!
//printf( "%.3f,%.3f,%.3f\n", rAnkleToToe.normalized().x(), rAnkleToToe.normalized().y(), rAnkleToToe.normalized().z() );
   // 1
   Eigen::Vector3d rToesVector = (rBigToe - rSmallToe).normalized();
   Eigen::Vector3d lToesVector = (lBigToe - lSmallToe).normalized();
//printf( "%.3f,%.3f,%.3f\n", rToesVector.x(), rToesVector.y(), rToesVector.z() );
   
   // 2
   //rToesVector = rAnkleToToe.normalized().cross( rToesVector ).normalized().cross( rAnkleToToe.normalized() ).normalized();
   //rToesVector = rAnkleToToe.normalized().cross( rToesVector ).normalized();
   rToesVector = rToesVector.cross( rAnkleToToe.normalized() ).normalized();
   //rToesVector = rAnkleToToe.normalized().cross( rToesVector ).normalized();
   //lToesVector = lAnkleToToe.normalized().cross( lToesVector ).normalized().cross( lAnkleToToe.normalized() ).normalized();
   lToesVector = lToesVector.cross( lAnkleToToe.normalized() ).normalized();
//printf( "%.3f,%.3f,%.3f\n", rToesVector.x(), rToesVector.y(), rToesVector.z() );
   
   // 3
   rToesVector = (rAnkleRestPoseAdjustment * rAnkleRotation).inverse()._transformVector( rToesVector );
   lToesVector = (lAnkleRestPoseAdjustment * lAnkleRotation).inverse()._transformVector( lToesVector );
//printf( "%.3f,%.3f,%.3f\n", rToesVector.x(), rToesVector.y(), rToesVector.z() );
//printf( "%.3f,%.3f,%.3f\n", lToesVector.x(), lToesVector.y(), lToesVector.z() );
//Eigen::Vector3d ankleUpVec = (rAnkleRestPoseAdjustment * rAnkleRotation)._transformVector( Eigen::Vector3d::UnitZ() );
//(void)ankleUpVec;
   
   // 4
   Eigen::Quaterniond rAnkleRoll = {1,0,0,0}, lAnkleRoll = {1,0,0,0};
   //Eigen::Quaterniond rAnkleRoll = Eigen::Quaterniond::FromTwoVectors( -Eigen::Vector3d::UnitZ(), rToesVector );
   //Eigen::Quaterniond lAnkleRoll = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitZ(), lToesVector );
   
//Eigen::AngleAxisd lToesRollAngle( rAnkleRoll );
//printf( "%.3f\n", lToesRollAngle.angle() * lToesRollAngle.axis()[1] );

   // Finish the ankles
   rig.rAnkle.length = rAnkleToToe.norm() * ankleToeRatio;
   rig.rAnkle.quaternion = Utility::QuaternionToRaw( rAnkleRotation * rAnkleRoll );
   rig.rAnkle.quaternionAbs = Utility::QuaternionToRaw( rAnkleRestPoseAdjustment * (rAnkleRotation * rAnkleRoll) );
   rig.lAnkle.length = lAnkleToToe.norm() * ankleToeRatio;
   rig.lAnkle.quaternion = Utility::QuaternionToRaw( lAnkleRotation * lAnkleRoll );
   rig.lAnkle.quaternionAbs = Utility::QuaternionToRaw( lAnkleRestPoseAdjustment * (lAnkleRotation * lAnkleRoll) );
   
   // Toe base (ball of foot)
   rig.rToeBase.length = rAnkleToToe.norm() * (1-ankleToeRatio);
   rig.rToeBase.quaternion = {0,0,0,1};
   rig.rToeBase.quaternionAbs = Utility::QuaternionToRaw( Utility::RawToQuaternion( rig.rAnkle.quaternionAbs ) * Utility::RawToQuaternion( restPose.rToeBase.quaternion ) );
   rig.lToeBase.length = lAnkleToToe.norm() * (1-ankleToeRatio);
   rig.lToeBase.quaternion = {0,0,0,1};
   rig.lToeBase.quaternionAbs = Utility::QuaternionToRaw( Utility::RawToQuaternion( rig.lAnkle.quaternionAbs ) * Utility::RawToQuaternion( restPose.lToeBase.quaternion ) );
   
   // Heel, big toe, and small toe are supplimentary joints
   _rigPose.SupplimentaryJoints.push_back( SupplimentaryJoint( "rightHeel",     "rightKnee", KpToRigHelper::CreateJoint( rig.rKnee, rHeel - rAnkle ) ) );
   _rigPose.SupplimentaryJoints.push_back( SupplimentaryJoint( "rightBigToe",   "rightKnee", KpToRigHelper::CreateJoint( rig.rKnee, rBigToe - rAnkle ) ) );
   _rigPose.SupplimentaryJoints.push_back( SupplimentaryJoint( "rightSmallToe", "rightKnee", KpToRigHelper::CreateJoint( rig.rKnee, rSmallToe - rAnkle ) ) );
   _rigPose.SupplimentaryJoints.push_back( SupplimentaryJoint( "leftHeel",      "leftKnee",  KpToRigHelper::CreateJoint( rig.lKnee, lHeel - lAnkle ) ) );
   _rigPose.SupplimentaryJoints.push_back( SupplimentaryJoint( "leftBigToe",    "leftKnee",  KpToRigHelper::CreateJoint( rig.lKnee, lBigToe - lAnkle ) ) );
   _rigPose.SupplimentaryJoints.push_back( SupplimentaryJoint( "leftSmallToe",  "leftKnee",  KpToRigHelper::CreateJoint( rig.lKnee, lSmallToe - lAnkle ) ) );
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
bool KpMpii_27::ValidateFeet() const
{
   return false;
//   Eigen::Quaterniond parentLocation = spine4Quaternion * Eigen::AngleAxisd( M_PI / 2, forwardVector );
//
//   // rShoulder
//   Eigen::Vector3d parentLocation = torsoLocation;
//   boneVector = Utility::RawToVector( rig.rShoulder.offset );
//   Eigen::Vector3d childLocation = parentLocation + boneVector;
//   Eigen::Vector3d kprShoulder = Utility::RawToVector( pose.Keypoint( RIGHT_SHOULDER ) );
//   distance = (childLocation - kprShoulder).norm();
//   if ( distance > tolerance )
//      return false;
//   parentLocation = childLocation;
//
//   // rElbow
//   q = rShoulderAdjustmentRotatation * Utility::RawToQuaternion( rig.rShoulder.quaternion );
//   double boneLength = rig.rShoulder.length;
//   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
//   childLocation = parentLocation + (boneVector*boneLength);
//   Eigen::Vector3d kprElbow = Utility::RawToVector( pose.Keypoint( RIGHT_ELBOW ) );
//   distance = (childLocation - kprElbow).norm();
//   if ( distance > tolerance )
//      return false;
//   if ( !q.isApprox( Utility::RawToQuaternion( rig.rShoulder.quaternionAbs ) ) )
//      return false;
//   parentLocation = childLocation;
//
//   return true;
}
