#define _USE_MATH_DEFINES
#include <cmath>
#include "RigPose.hpp"
#include "Utility.hpp"

RigPose::RigPose( int timestamp, const Rig & rig )
   :_timestamp( timestamp ),
   _rig( rig )
{
}
RigPose RigPose::Interpolate( const RigPose & rhs, double ratio ) const
{
   // Start by copying this
   RigPose returnValue( *this );
      
   // LERP the position
   Eigen::Vector3d position1 = Utility::RawToVector( this->_rig.location );
   Eigen::Vector3d position2 = Utility::RawToVector( rhs._rig.location );
   Eigen::Vector3d vec = position2 - position1;
   vec *= ratio;
   Eigen::Vector3d interpolatedPosition = position1 + vec;
   returnValue.GetRig().location = Utility::VectorToRaw( interpolatedPosition );

   // Bone lengths are the same and already copied

   // SLERP all bone rotations in the final rig
   for ( int i = 0; i < _rig.numJointsUsed; ++i )
   {
      Eigen::Quaterniond rotation1 = Utility::RawToQuaternion( this->_rig.GetJoint( Rig::JOINT_TYPE(i) ).quaternion );
      Eigen::Quaterniond rotation2 = Utility::RawToQuaternion( rhs._rig.GetJoint( Rig::JOINT_TYPE(i) ).quaternion );
      Eigen::Quaterniond interpolatedRotation = rotation1.slerp( ratio, rotation2 );
      returnValue.GetRig().GetJoint( Rig::JOINT_TYPE(i) ).quaternion = Utility::QuaternionToRaw( interpolatedRotation );
   }

   // SLERP all supplimentary joints
   for ( size_t i = 0; i < _supplimentaryJoints.size(); ++i )
   {
      Eigen::Quaterniond rotation1 = Utility::RawToQuaternion( this->_supplimentaryJoints[i].second.quaternion );
      Eigen::Quaterniond rotation2 = Utility::RawToQuaternion( rhs._supplimentaryJoints[i].second.quaternion );
      Eigen::Quaterniond interpolatedRotation = rotation1.slerp( ratio, rotation2 );
      returnValue._supplimentaryJoints[i].second.quaternion = Utility::QuaternionToRaw( interpolatedRotation );
   }

   // LERP all bone offsets
   for ( int i = 0; i < _rig.numJointOffsetsUsed; ++i )
   {
      Eigen::Vector3d offset1 = Utility::RawToVector( this->_rig.GetJointOffset( Rig::JOINT_OFFSET_TYPE(i) ) );
      Eigen::Vector3d offset2 = Utility::RawToVector( rhs._rig.GetJointOffset( Rig::JOINT_OFFSET_TYPE(i) ) );
      Eigen::Vector3d diffVector = offset2 - offset1;
      diffVector *= ratio;
      Eigen::Vector3d interpolatedOffset = offset1 + diffVector;
      returnValue.GetRig().GetJointOffset( Rig::JOINT_OFFSET_TYPE(i) ) = Utility::VectorToRaw( interpolatedOffset );
   }
   
   // Ensure our absolute rotations are set properly
   returnValue.UpdateAbsRotations();
   
   return returnValue;
};
void RigPose::UpdateAbsRotations()
{
   Eigen::Vector3d upVector( 0, 1, 0 );
   //Eigen::Vector3d downVector( 0, -1, 0 );
   Eigen::Quaterniond pelvisQuaternion = Utility::RawToQuaternion( _rig.pelvis.quaternion );
   Eigen::Quaterniond q;

   // SPINE
   {
      // pelvis/spine1
      q = pelvisQuaternion;
      _rig.pelvis.quaternionAbs = Utility::QuaternionToRaw( q );
      
      // spine2
      q = q * Utility::RawToQuaternion( _rig.spine2.quaternion );
      _rig.spine2.quaternionAbs = Utility::QuaternionToRaw( q );
      
      // spine3
      q = q * Utility::RawToQuaternion( _rig.spine3.quaternion );
      _rig.spine3.quaternionAbs = Utility::QuaternionToRaw( q );

      // spine4
      q = q * Utility::RawToQuaternion( _rig.spine4.quaternion );
      _rig.spine4.quaternionAbs = Utility::QuaternionToRaw( q );

      // torso
      q = q * Utility::RawToQuaternion( _rig.baseNeck.quaternion );
      _rig.baseNeck.quaternionAbs = Utility::QuaternionToRaw( q );

      // baseHead
      q = q * Utility::RawToQuaternion( _rig.baseHead.quaternion );
      _rig.baseHead.quaternionAbs = Utility::QuaternionToRaw( q );
   }
   
   // LEGS
   {
      Eigen::Quaterniond hipAdjustmentRotation( Eigen::AngleAxisd( M_PI, Eigen::Vector3d::UnitZ() ) );
      Eigen::Quaterniond ankleAdjustementRotation( Eigen::AngleAxisd( M_PI * 7.0/18.0, Eigen::Vector3d::UnitX() ) );
      Eigen::Quaterniond toeBaseAdjustementRotation( Eigen::AngleAxisd( M_PI / 9.0, Eigen::Vector3d::UnitX() ) );
      
      // rHip
      q = (pelvisQuaternion * hipAdjustmentRotation) * Utility::RawToQuaternion( _rig.rHip.quaternion );
      _rig.rHip.quaternionAbs = Utility::QuaternionToRaw( q );

      // rKnee
      q = q * Utility::RawToQuaternion( _rig.rKnee.quaternion );
      _rig.rKnee.quaternionAbs = Utility::QuaternionToRaw( q );
      
      // rAnkle
      q = (q * ankleAdjustementRotation) * Utility::RawToQuaternion( _rig.rAnkle.quaternion );
      _rig.rAnkle.quaternionAbs = Utility::QuaternionToRaw( q );
      
      // rToeBase
      q = (q * toeBaseAdjustementRotation) * Utility::RawToQuaternion( _rig.rToeBase.quaternion );
      _rig.rToeBase.quaternionAbs = Utility::QuaternionToRaw( q );
      
      // lHip
      q = (pelvisQuaternion * hipAdjustmentRotation) * Utility::RawToQuaternion( _rig.lHip.quaternion );
      _rig.lHip.quaternionAbs = Utility::QuaternionToRaw( q );

      // lKnee
      q = q * Utility::RawToQuaternion( _rig.lKnee.quaternion );
      _rig.lKnee.quaternionAbs = Utility::QuaternionToRaw( q );
      
      // lAnkle
      q = (q * ankleAdjustementRotation) * Utility::RawToQuaternion( _rig.lAnkle.quaternion );
      _rig.lAnkle.quaternionAbs = Utility::QuaternionToRaw( q );
      
      // lToeBase
      q = (q * toeBaseAdjustementRotation) * Utility::RawToQuaternion( _rig.lToeBase.quaternion );
      _rig.lToeBase.quaternionAbs = Utility::QuaternionToRaw( q );
   }
   
   // ARMS
   {
      Eigen::Quaterniond spine4Quaternion = Utility::RawToQuaternion( _rig.spine4.quaternionAbs );
      Eigen::Quaterniond rShoulderAdjustmentRotatation( Eigen::AngleAxisd(  M_PI / 2, Eigen::Vector3d::UnitZ() ) );
      Eigen::Quaterniond lShoulderAdjustmentRotatation( Eigen::AngleAxisd( -M_PI / 2, Eigen::Vector3d::UnitZ() ) );

      // rShoulder
      q = (spine4Quaternion * rShoulderAdjustmentRotatation) * Utility::RawToQuaternion( _rig.rShoulder.quaternion );
      _rig.rShoulder.quaternionAbs = Utility::QuaternionToRaw( q );

      // rElbow
      q = q * Utility::RawToQuaternion( _rig.rElbow.quaternion );
      _rig.rElbow.quaternionAbs = Utility::QuaternionToRaw( q );
      
      // rWrist
      q = q * Utility::RawToQuaternion( _rig.rWrist.quaternion );
      _rig.rWrist.quaternionAbs = Utility::QuaternionToRaw( q );
      
      // lShoulder
      q = (spine4Quaternion * lShoulderAdjustmentRotatation) * Utility::RawToQuaternion( _rig.lShoulder.quaternion );
      _rig.lShoulder.quaternionAbs = Utility::QuaternionToRaw( q );

      // lElbow
      q = q * Utility::RawToQuaternion( _rig.lElbow.quaternion );
      _rig.lElbow.quaternionAbs = Utility::QuaternionToRaw( q );
      
      // lWrist
      q = q * Utility::RawToQuaternion( _rig.lWrist.quaternion );
      _rig.lWrist.quaternionAbs = Utility::QuaternionToRaw( q );
   }
}
