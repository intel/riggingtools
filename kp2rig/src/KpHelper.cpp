#define _USE_MATH_DEFINES
#include <cmath>
#include "KpHelper.hpp"
#include "Utility.hpp"
#include "Pose.hpp"
#include "RigPose.hpp"

bool KpHelper::ValidateSpine( const Pose & pose )
{
   const double tolerance = 0.25;
   double distance;
   const Rig & rig = pose.RigPose().GetRig();
   Eigen::Vector3d rigPelvis = Utility::RawToVector( rig.location );
   Eigen::Quaterniond pelvisQuaternion = Utility::RawToQuaternion( rig.pelvis.quaternion );
   Eigen::Quaterniond q;
   Eigen::Vector3d boneVector;

   // Cannot validate the pelvis because it's generated from the hips and may difffer then the original

   // spine2 location (nothing to compare against, but we need to calculate it)
   boneVector = pelvisQuaternion._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.pelvis.length;
   Eigen::Vector3d spine2Head = rigPelvis + boneVector;
   if ( !pelvisQuaternion.isApprox( Utility::RawToQuaternion( rig.pelvis.quaternionAbs ) ) )
      return false;

   // spine3 location (nothing to compare against, but we need to calculate it)
   q = (pelvisQuaternion * Utility::RawToQuaternion( rig.spine2.quaternion ));
   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.spine2.length;
   Eigen::Vector3d spine3Head = spine2Head + boneVector;
   if ( !q.isApprox( Utility::RawToQuaternion( rig.spine2.quaternionAbs ) ) )
      return false;

   // spine4 location (nothing to compare against, but we need to calculate it)
   q = (q * Utility::RawToQuaternion( rig.spine3.quaternion ));
   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.spine3.length;
   Eigen::Vector3d spine4Head = spine3Head + boneVector;
   if ( !q.isApprox( Utility::RawToQuaternion( rig.spine3.quaternionAbs ) ) )
      return false;

   // baseNeck
   q = (q * Utility::RawToQuaternion( rig.spine4.quaternion ));
   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.spine4.length;
   Eigen::Vector3d rigBaseNeck = spine4Head + boneVector;
   Eigen::Vector3d kpBaseNeck = Utility::RawToVector( pose.Keypoint( BASE_NECK ) );
   distance = (rigBaseNeck - kpBaseNeck).norm();
   if ( distance > tolerance )
      return false;
   if ( !q.isApprox( Utility::RawToQuaternion( rig.spine4.quaternionAbs ) ) )
      return false;

   // baseHead
   q = (q * Utility::RawToQuaternion( rig.baseNeck.quaternion ));
   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.baseNeck.length;
   Eigen::Vector3d rigNeck = rigBaseNeck + boneVector;
   if ( pose.HasKeypoint( BASE_HEAD) )
   {
      Eigen::Vector3d kpNeck = Utility::RawToVector( pose.Keypoint( BASE_HEAD ) );
      distance = (rigNeck - kpNeck).norm();
      if ( distance > tolerance )
         return false;
      if ( !q.isApprox( Utility::RawToQuaternion( rig.baseNeck.quaternionAbs ) ) )
         return false;
   }

   // top of head
   q = (q * Utility::RawToQuaternion( rig.baseHead.quaternion ));
   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.baseHead.length;
   Eigen::Vector3d rigHead = rigNeck + boneVector;
   Eigen::Vector3d kpHead = Utility::RawToVector( pose.Keypoint( TOP_HEAD ) );
   distance = (rigHead - kpHead).norm();
   if ( distance > tolerance )
      return false;
   if ( !q.isApprox( Utility::RawToQuaternion( rig.baseHead.quaternionAbs ) ) )
      return false;

   return true;
}
bool KpHelper::ValidateLegs( const Pose & pose )
{
   constexpr double tolerance = 0.25;
   double distance;
   const Rig & rig = pose.RigPose().GetRig();
   Eigen::Vector3d forwardVector( 0, 0, 1 );
   Eigen::Vector3d rigPelvis = Utility::RawToVector( rig.location );
   Eigen::Quaterniond pelvisQuaternion = Utility::RawToQuaternion( rig.pelvis.quaternion );
   Eigen::Quaterniond hipAdjustmentRotation( Eigen::AngleAxisd( M_PI, forwardVector ) );
   Eigen::Quaterniond q;
   Eigen::Vector3d boneVector;

   // rHip
   Eigen::Vector3d rigrHip = rigPelvis + Utility::RawToVector( rig.rHip.offset );
   Eigen::Vector3d kprHip = Utility::RawToVector( pose.Keypoint( RIGHT_HIP ) );
   distance = (rigrHip - kprHip).norm();
   if ( distance > tolerance )
      return false;

   // rKnee
   q = (pelvisQuaternion * hipAdjustmentRotation) * Utility::RawToQuaternion( rig.rHip.quaternion );
   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.rHip.length;
   Eigen::Vector3d rigrKnee = rigrHip + boneVector;
   Eigen::Vector3d kprKnee = Utility::RawToVector( pose.Keypoint( RIGHT_KNEE ) );
   distance = (rigrKnee - kprKnee).norm();
   if ( distance > tolerance )
      return false;

   // rAnkle
   q = q * Utility::RawToQuaternion( rig.rKnee.quaternion );
   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.rKnee.length;
   Eigen::Vector3d rigrAnkle = rigrKnee + boneVector;
   Eigen::Vector3d kprAnkle = Utility::RawToVector( pose.Keypoint( RIGHT_ANKLE ) );
   distance = (rigrAnkle - kprAnkle).norm();
   if ( distance > tolerance )
      return false;

   // lHip
   Eigen::Vector3d riglHip = rigPelvis + Utility::RawToVector( rig.lHip.offset );
   Eigen::Vector3d kplHip = Utility::RawToVector( pose.Keypoint( LEFT_HIP ) );
   distance = (riglHip - kplHip).norm();
   if ( distance > tolerance )
      return false;

   // lKnee
   q = (pelvisQuaternion * hipAdjustmentRotation) * Utility::RawToQuaternion( rig.lHip.quaternion );
   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.lHip.length;
   Eigen::Vector3d riglKnee = riglHip + boneVector;
   Eigen::Vector3d kplKnee = Utility::RawToVector( pose.Keypoint( LEFT_KNEE ) );
   distance = (riglKnee - kplKnee).norm();
   if ( distance > tolerance )
      return false;

   // rAnkle
   q = q * Utility::RawToQuaternion( rig.lKnee.quaternion );
   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.lKnee.length;
   Eigen::Vector3d riglAnkle = riglKnee + boneVector;
   Eigen::Vector3d kplAnkle = Utility::RawToVector( pose.Keypoint( LEFT_ANKLE ) );
   distance = (riglAnkle - kplAnkle).norm();
   if ( distance > tolerance )
      return false;

   return true;
}
bool KpHelper::ValidateArms( const Pose & pose )
{
   const double tolerance = 0.25;
   double distance;
   const Rig & rig = pose.RigPose().GetRig();
   Eigen::Vector3d forwardVector( 0, 0, 1 );

   Eigen::Vector3d torsoLocation = Utility::RawToVector( rig.location );
   Eigen::Quaterniond q = Utility::RawToQuaternion( rig.pelvis.quaternion );
   Eigen::Vector3d boneVector;
   
   // Walk from the pelvis up to the tip of spine4 to get the location of the torso
   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.pelvis.length;
   torsoLocation += boneVector;
   
   q = q * Utility::RawToQuaternion( rig.spine2.quaternion );
   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.spine2.length;
   torsoLocation += boneVector;

   q = q * Utility::RawToQuaternion( rig.spine3.quaternion );
   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.spine3.length;
   torsoLocation += boneVector;
   
   q = q * Utility::RawToQuaternion( rig.spine4.quaternion );
   boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
   boneVector *= rig.spine4.length;
   torsoLocation += boneVector;
   
   Eigen::Quaterniond spine4Quaternion = q;
   
   // RIGHT ARM
   {
      Eigen::Quaterniond rShoulderAdjustmentRotatation = spine4Quaternion * Eigen::AngleAxisd( M_PI / 2, forwardVector );
      
      // rShoulder
      Eigen::Vector3d parentLocation = torsoLocation;
      boneVector = Utility::RawToVector( rig.rShoulder.offset );
      Eigen::Vector3d childLocation = parentLocation + boneVector;
      Eigen::Vector3d kprShoulder = Utility::RawToVector( pose.Keypoint( RIGHT_SHOULDER ) );
      distance = (childLocation - kprShoulder).norm();
      if ( distance > tolerance )
         return false;
      parentLocation = childLocation;

      // rElbow
      q = rShoulderAdjustmentRotatation * Utility::RawToQuaternion( rig.rShoulder.quaternion );
      double boneLength = rig.rShoulder.length;
      boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
      childLocation = parentLocation + (boneVector*boneLength);
      Eigen::Vector3d kprElbow = Utility::RawToVector( pose.Keypoint( RIGHT_ELBOW ) );
      distance = (childLocation - kprElbow).norm();
      if ( distance > tolerance )
         return false;
      if ( !q.isApprox( Utility::RawToQuaternion( rig.rShoulder.quaternionAbs ) ) )
         return false;
      parentLocation = childLocation;

      // rWrist
      q = q * Utility::RawToQuaternion( rig.rElbow.quaternion );
      boneLength = rig.rElbow.length;
      boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
      childLocation = parentLocation + (boneVector*boneLength);
      Eigen::Vector3d kprWrist = Utility::RawToVector( pose.Keypoint( RIGHT_WRIST ) );
      distance = (childLocation - kprWrist).norm();
      if ( distance > tolerance )
         return false;
      if ( !q.isApprox( Utility::RawToQuaternion( rig.rElbow.quaternionAbs ) ) )
         return false;
   }
   
   // LEFT ARM
   {
      Eigen::Quaterniond lShoulderAdjustmentRotatation = spine4Quaternion * Eigen::AngleAxisd( -M_PI / 2, forwardVector );
      
      // lShoulder
      Eigen::Vector3d parentLocation = torsoLocation;
      boneVector = Utility::RawToVector( rig.lShoulder.offset );
      Eigen::Vector3d childLocation = parentLocation + boneVector;
      Eigen::Vector3d kplShoulder = Utility::RawToVector( pose.Keypoint( LEFT_SHOULDER ) );
      distance = (childLocation - kplShoulder).norm();
      if ( distance > tolerance )
         return false;
      parentLocation = childLocation;

      // lElbow
      q = lShoulderAdjustmentRotatation * Utility::RawToQuaternion( rig.lShoulder.quaternion );
      double boneLength = rig.lShoulder.length;
      boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
      childLocation = parentLocation + (boneVector*boneLength);
      Eigen::Vector3d kplElbow = Utility::RawToVector( pose.Keypoint( LEFT_ELBOW ) );
      distance = (childLocation - kplElbow).norm();
      if ( distance > tolerance )
         return false;
      if ( !q.isApprox( Utility::RawToQuaternion( rig.lShoulder.quaternionAbs ) ) )
         return false;
      parentLocation = childLocation;
      
      // lWrist
      q = q * Utility::RawToQuaternion( rig.lElbow.quaternion );
      boneLength = rig.lElbow.length;
      boneVector = q._transformVector( Eigen::Vector3d::UnitY() );
      childLocation = parentLocation + (boneVector*boneLength);
      Eigen::Vector3d kplWrist = Utility::RawToVector( pose.Keypoint( LEFT_WRIST ) );
      distance = (childLocation - kplWrist).norm();
      if ( distance > tolerance )
         return false;
      if ( !q.isApprox( Utility::RawToQuaternion( rig.lElbow.quaternionAbs ) ) )
         return false;
   }
   return true;
}
void KpHelper::KeypointsRhToLh( Pose & pose )
{
   std::swap( pose.Keypoint( RIGHT_ANKLE ),    pose.Keypoint( LEFT_ANKLE ) );
   std::swap( pose.Keypoint( RIGHT_KNEE ),     pose.Keypoint( LEFT_KNEE ) );
   std::swap( pose.Keypoint( RIGHT_HIP ),      pose.Keypoint( LEFT_HIP ) );
   std::swap( pose.Keypoint( RIGHT_WRIST ),    pose.Keypoint( LEFT_WRIST ) );
   std::swap( pose.Keypoint( RIGHT_ELBOW ),    pose.Keypoint( LEFT_ELBOW ) );
   std::swap( pose.Keypoint( RIGHT_SHOULDER ), pose.Keypoint( LEFT_SHOULDER ) );
}
