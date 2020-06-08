#include "RigToKpHelper.hpp"
#include "Pose.hpp"
#include "RigPose.hpp"
#include "Utility.hpp"

void RigToKpHelper::HandleSpine( Pose & pose )
{
   Rig & rig = pose.RigPose().GetRig();
   
   // pelvis
   pose.Keypoint( rig.location, PELVIS );
   
   // spine2
   Eigen::Vector3d parentLocation = Utility::RawToVector( rig.location );
   double boneLength = rig.pelvis.length;
   Eigen::Vector3d boneVector = Utility::RawToQuaternion( rig.pelvis.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   parentLocation = parentLocation + (boneVector*boneLength);
   
   // spine3
   boneLength = rig.spine2.length;
   boneVector = Utility::RawToQuaternion( rig.spine2.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   parentLocation = parentLocation + (boneVector*boneLength);
   
   // spine4
   boneLength = rig.spine3.length;
   boneVector = Utility::RawToQuaternion( rig.spine3.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   parentLocation = parentLocation + (boneVector*boneLength);
   
   // baseNeck
   boneLength = rig.spine4.length;
   boneVector = Utility::RawToQuaternion( rig.spine4.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   Eigen::Vector3d childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint(Utility::VectorToRaw( childLocation ), BASE_NECK );
   parentLocation = childLocation;
   
   // baseHead
   boneLength = rig.baseNeck.length;
   boneVector = Utility::RawToQuaternion( rig.baseNeck.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), BASE_HEAD );
   parentLocation = childLocation;
   
   // topHead
   boneLength = rig.baseHead.length;
   boneVector = Utility::RawToQuaternion( rig.baseHead.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), TOP_HEAD );
}
void RigToKpHelper::HandleLegs( Pose & pose )
{
   Rig & rig = pose.RigPose().GetRig();
   
   // left hip
   Eigen::Vector3d parentLocation = Utility::RawToVector( rig.location );
   double boneLength = Utility::RawToVector( rig.lHip.offset ).norm();
   Eigen::Vector3d boneVector = Utility::RawToQuaternion( rig.pelvis.quaternionAbs )._transformVector( Eigen::Vector3d::UnitX() );
   Eigen::Vector3d childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), LEFT_HIP );
   parentLocation = childLocation;
   
   // left knee
   boneLength = rig.lHip.length;
   boneVector = Utility::RawToQuaternion( rig.lHip.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), LEFT_KNEE );
   parentLocation = childLocation;
   
   // left ankle
   boneLength = rig.lKnee.length;
   boneVector = Utility::RawToQuaternion( rig.lKnee.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), LEFT_ANKLE );
   
   // right hip
   parentLocation = Utility::RawToVector(rig.location);
   boneLength = Utility::RawToVector( rig.rHip.offset ).norm();
   boneVector = Utility::RawToQuaternion( rig.pelvis.quaternionAbs )._transformVector( -Eigen::Vector3d::UnitX() );
   childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), RIGHT_HIP );
   parentLocation = childLocation;
   
   // right knee
   boneLength = rig.rHip.length;
   boneVector = Utility::RawToQuaternion( rig.rHip.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), RIGHT_KNEE );
   parentLocation = childLocation;
   
   // right ankle
   boneLength = rig.rKnee.length;
   boneVector = Utility::RawToQuaternion( rig.rKnee.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), RIGHT_ANKLE );
}
void RigToKpHelper::HandleArms( Pose & pose )
{
   Rig & rig = pose.RigPose().GetRig();
   
   // This assumes baseNeck keypoint is already set!

   // left shoulder
   Eigen::Vector3d parentLocation = Utility::RawToVector( pose.Keypoint( BASE_NECK ) );
   double boneLength = Utility::RawToVector( rig.lShoulder.offset ).norm();
   Eigen::Vector3d boneVector = Utility::RawToQuaternion( rig.baseNeck.quaternionAbs )._transformVector( Eigen::Vector3d::UnitX() );
   Eigen::Vector3d childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), LEFT_SHOULDER );
   parentLocation = childLocation;
   
   // left elbow
   boneLength = rig.lShoulder.length;
   boneVector = Utility::RawToQuaternion( rig.lShoulder.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), LEFT_ELBOW );
   parentLocation = childLocation;
   
   // left wrist
   boneLength = rig.lElbow.length;
   boneVector = Utility::RawToQuaternion( rig.lElbow.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), LEFT_WRIST );
   
   // right shoulder
   parentLocation = Utility::RawToVector( pose.Keypoint( BASE_NECK ) );
   boneLength = Utility::RawToVector( rig.rShoulder.offset ).norm();
   boneVector = Utility::RawToQuaternion( rig.baseNeck.quaternionAbs )._transformVector( -Eigen::Vector3d::UnitX() );
   childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), RIGHT_SHOULDER );
   parentLocation = childLocation;
   
   // right elbow
   boneLength = rig.rShoulder.length;
   boneVector = Utility::RawToQuaternion( rig.rShoulder.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), RIGHT_ELBOW );
   parentLocation = childLocation;
   
   // right wrist
   boneLength = rig.rElbow.length;
   boneVector = Utility::RawToQuaternion( rig.rElbow.quaternionAbs )._transformVector( Eigen::Vector3d::UnitY() );
   childLocation = parentLocation + (boneVector*boneLength);
   pose.Keypoint( Utility::VectorToRaw( childLocation ), RIGHT_WRIST );
}
