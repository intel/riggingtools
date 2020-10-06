#define _USE_MATH_DEFINES
#include <cmath>
#include "KpToRigHelper.hpp"
#include "Pose.hpp"
#include "Utility.hpp"
#include "RigPose.hpp"

void KpToRigHelper::HandleSpine( Pose & pose )
{
   Rig & rig = pose.RigPose().GetRig();

   Eigen::Quaterniond q1;
   Eigen::Vector3d leftHip;
   Eigen::Vector3d rightHip;
   Eigen::Vector3d baseNeck;
   Eigen::Vector3d baseHead;
   Eigen::Vector3d topHead;
   Eigen::Vector3d leftShoulder;
   Eigen::Vector3d rightShoulder;
   
   if ( pose.HasKeypoint( LEFT_HIP ) )
   {
      auto & temp1 = pose.Keypoint( LEFT_HIP );
      leftHip = { temp1[0], temp1[1], temp1[2] };
      auto & temp2 = pose.Keypoint( RIGHT_HIP );
      rightHip = { temp2[0], temp2[1], temp2[2] };
   }
   else
   {
      throw std::runtime_error( "Hip keypoints are required" );
   }
   
   if ( pose.HasKeypoint( LEFT_SHOULDER ) )
   {
      auto & temp1 = pose.Keypoint( LEFT_SHOULDER );
      leftShoulder = { temp1[0], temp1[1], temp1[2] };
      auto & temp2 = pose.Keypoint( RIGHT_SHOULDER );
      rightShoulder = { temp2[0], temp2[1], temp2[2] };
   }
   else
   {
      throw std::runtime_error( "Shoulder keypoints are required" );
   }
   
   if ( pose.HasKeypoint( BASE_NECK ) )
   {
      auto & temp = pose.Keypoint( BASE_NECK );
      baseNeck = { temp[0], temp[1], temp[2] };
   }
   else
   {
      // Use the shoulders to guess this
      Eigen::Vector3d shoulderVector = leftShoulder - rightShoulder;
      shoulderVector *= 0.5;
      Eigen::Vector3d baseNeckLocation = rightShoulder + shoulderVector;
      baseNeck = { baseNeckLocation[0], baseNeckLocation[1], baseNeckLocation[2] };
   }
   
   if ( pose.HasKeypoint( TOP_HEAD ) )
   {
      auto & temp = pose.Keypoint( TOP_HEAD );
      topHead = { temp[0], temp[1], temp[2] };
   }
   else
   {
      // TODO: Figure out how to use other info to estimate this
      throw std::runtime_error( "Top of head keypoint is required (for now)" );
   }
   
   if ( pose.HasKeypoint( BASE_HEAD ) )
   {
      auto & temp = pose.Keypoint( BASE_HEAD );
      baseHead = { temp[0], temp[1], temp[2] };
   }
   else
   {
      // Assume the neck is straight and length is a 30/70 split between
      // base of neck and top of head
      Eigen::Vector3d neckVector = topHead - baseNeck;
      neckVector *= 0.3;
      Eigen::Vector3d baseHeadLocation = baseNeck + neckVector;
      baseHead = { baseHeadLocation[0], baseHeadLocation[1], baseHeadLocation[2] };
   }

   // Useful vectors
   Eigen::Vector3d upVector( 0, 1, 0 );
   Eigen::Vector3d forwardVector( 0, 0, 1 );
   Eigen::Vector3d hipsVector = leftHip - rightHip;
   Eigen::Vector3d shoulderVector = leftShoulder - rightShoulder;
   Eigen::Vector3d torsoVector = baseHead - baseNeck;
   Eigen::Vector3d neckVector = topHead - baseHead;
   
   // Pelvis (or spine1):
   // The pelvis (same as spine1) is the parent of all joints, so it is important to get this correct!
   // The pelvis is the only joint with an absolute position
   //
   // The pelvis point is ignored; instead we are given a vector between the hips:
   //  1 Place the pelvis in the center of the hips vector
   //  2 Set the absolute position of the pelvis
   //  3 The number of spine joints provided is 3 (pelvis --> baseNeck --> baseHead),
   //    but we want to output 3 more joints inbetween the pelvis and baseNeck and introduce a "natural" bending of the spine.
   //    Create new spine points along a rational quadratic bezier curve where:
   //       p0 is the pelvis
   //       p1 is a calculated control point
   //       p2 is the baseNeck
   //    We need to determine good control points and then create the new spine joints:
   //       a get the a from the base of the head to the baseNeck, then double-cross this with the shoulder vector to get it square with the shoulders
   //       b scale this vector so it's half the length of the baseNeck-pelvis vector
   //       c determine our control points
   //       d create the additional spine points along the bezier curve
   //       e set our pelvis/spine1 unit vector to the first spine point along the curve
   //  4 Determine the pelvis quaternion without any roll correction
   //  5 Determine the roll of the pelvis
   //     a calculate the pelvis forward vector and apply the inverse pelvis quaternion so that we are guaranteed to have
   //       this forward vector lie on the XZ plane.
   //       Note that prior to applying the inverse pelvis quaternion, this forward vector is perpendicular to the hips vector
   //       and does NOT bend left/right. Any left/right bends will be accounted for in spine2-4.
   //     b calculate the roll quaternion of the pelvis/spine1
   //  6 Apply the final quaternion
   //  7 Scale the spine1/pelvis length. spine2-4 will complete the remaining distance.
   
   // 1)
   Eigen::Vector3d pelvis( rightHip[0] + hipsVector[0]/2, rightHip[1] + hipsVector[1]/2, rightHip[2] + hipsVector[2]/2 );
   
   // 2
   rig.location = { pelvis[0], pelvis[1], pelvis[2] };
   
   // 3a
   Eigen::Vector3d downwardHeadVector = (baseNeck - topHead).normalized();
   Eigen::Vector3d thoraxForwardVector = shoulderVector.normalized().cross( downwardHeadVector );
   downwardHeadVector = thoraxForwardVector.normalized().cross( shoulderVector.normalized() );
   
   // 3b
   downwardHeadVector *= ((baseNeck - pelvis).norm() / 2);
   
   // 3c
   Eigen::Vector3d p0 = pelvis;
   Eigen::Vector3d p1 = baseNeck + downwardHeadVector;
   Eigen::Vector3d p2 = baseNeck;
   
   // 3d
   std::vector< Eigen::Vector3d > interpolatedSpinePoints = Utility::QuadraticBezierCurve( p0,
      p1,
      p2,
      3,
      0.3 );
      
   // 3e
   Eigen::Vector3d spine1UnitVector = (interpolatedSpinePoints[0] - pelvis).normalized();
   
   // 4
   q1 = Eigen::Quaterniond::FromTwoVectors( upVector, spine1UnitVector );
   
   // 5a
   Eigen::Vector3d pelvisForwardVector = hipsVector.normalized().cross( spine1UnitVector );
   pelvisForwardVector = q1.inverse()._transformVector( pelvisForwardVector.normalized() );

   // 5b
   Eigen::Quaterniond pelvisRoll = Eigen::Quaterniond::FromTwoVectors( forwardVector, pelvisForwardVector );
   
   // 6
   q1 = q1 * pelvisRoll;
   rig.spine1.quaternion = Utility::QuaternionToRaw( q1 );
   rig.spine1.quaternionAbs = rig.spine1.quaternion;
   
   // 7
   rig.spine1.length = (interpolatedSpinePoints[0] - pelvis).norm();
   
   
   // All subsequent rotations must be RELATIVE, so we need to invert the cumulative quaternion before
   // finding the difference-of-vectors.
        
   // spine2
   Eigen::Vector3d spine2UnitVector = (interpolatedSpinePoints[1] - interpolatedSpinePoints[0]).normalized();
   rig.spine2.length = (interpolatedSpinePoints[1] - interpolatedSpinePoints[0]).norm();
   Eigen::Quaterniond relativeRotation = Eigen::Quaterniond::FromTwoVectors( upVector, q1.inverse()._transformVector( spine2UnitVector ) );
   rig.spine2.quaternion = Utility::QuaternionToRaw( relativeRotation );
   rig.spine2.quaternionAbs = Utility::QuaternionToRaw( q1 * relativeRotation );
   q1 = Utility::RawToQuaternion( rig.spine2.quaternionAbs );
   
   // spine3
   Eigen::Vector3d spine3UnitVector = (interpolatedSpinePoints[2] - interpolatedSpinePoints[1]).normalized();
   rig.spine3.length = (interpolatedSpinePoints[2] - interpolatedSpinePoints[1]).norm();
   relativeRotation = Eigen::Quaterniond::FromTwoVectors( upVector, q1.inverse()._transformVector( spine3UnitVector ) );
   rig.spine3.quaternion = Utility::QuaternionToRaw( relativeRotation );
   rig.spine3.quaternionAbs = Utility::QuaternionToRaw( q1 * relativeRotation );
   q1 = Utility::RawToQuaternion( rig.spine3.quaternionAbs );
   
   // spine4
   Eigen::Vector3d spine4UnitVector = (baseNeck - interpolatedSpinePoints[2]).normalized();
   rig.spine4.length = (baseNeck - interpolatedSpinePoints[2]).norm();
   relativeRotation = Eigen::Quaterniond::FromTwoVectors( upVector, q1.inverse()._transformVector( spine4UnitVector ) );
   rig.spine4.quaternion = Utility::QuaternionToRaw( relativeRotation );
   rig.spine4.quaternionAbs = Utility::QuaternionToRaw( q1 * relativeRotation );
   q1 = Utility::RawToQuaternion( rig.spine4.quaternionAbs );
   
   // Torso (which is really the bottom-baseHead joint)
   rig.baseNeck.length = torsoVector.norm();
   relativeRotation = Eigen::Quaterniond::FromTwoVectors( upVector, q1.inverse()._transformVector( torsoVector ) );
   rig.baseNeck.quaternion = Utility::QuaternionToRaw( relativeRotation );
   rig.baseNeck.quaternionAbs = Utility::QuaternionToRaw( q1 * relativeRotation );
   q1 = Utility::RawToQuaternion( rig.baseNeck.quaternionAbs );
   
   // Neck
   rig.baseHead.length = neckVector.norm();
   relativeRotation = Eigen::Quaterniond::FromTwoVectors( upVector, q1.inverse()._transformVector( neckVector ) );
   rig.baseHead.quaternion = Utility::QuaternionToRaw( relativeRotation );
   rig.baseHead.quaternionAbs = Utility::QuaternionToRaw( q1 * relativeRotation );
   q1 = Utility::RawToQuaternion( rig.baseHead.quaternionAbs );
}
void KpToRigHelper::HandleLegs( Pose & pose )
{
   Rig & rig = pose.RigPose().GetRig();

   Eigen::Quaterniond pelvisQuaternion = Utility::RawToQuaternion( rig.pelvis.quaternion );
   Eigen::Vector3d rHip;
   Eigen::Vector3d rKnee;
   Eigen::Vector3d rAnkle;
   Eigen::Vector3d lHip;
   Eigen::Vector3d lKnee;
   Eigen::Vector3d lAnkle;
   const double BEND_THRESHOLD = M_PI / 12;
   
   if ( pose.HasKeypoint( LEFT_HIP ) )
   {
      auto & temp1 = pose.Keypoint( LEFT_HIP );
      lHip = { temp1[0], temp1[1], temp1[2] };
      auto & temp2 = pose.Keypoint( RIGHT_HIP );
      rHip = { temp2[0], temp2[1], temp2[2] };
   }
   else
   {
      throw std::runtime_error( "Hip keypoints are required" );
   }
   
   if ( pose.HasKeypoint( LEFT_KNEE ) )
   {
      auto & temp1 = pose.Keypoint( LEFT_KNEE );
      lKnee = { temp1[0], temp1[1], temp1[2] };
      auto & temp2 = pose.Keypoint( RIGHT_KNEE );
      rKnee = { temp2[0], temp2[1], temp2[2] };
   }
   else
   {
      throw std::runtime_error( "Knee keypoints are required" );
   }
   
   if ( pose.HasKeypoint( LEFT_ANKLE ) )
   {
      auto & temp1 = pose.Keypoint( LEFT_ANKLE );
      lAnkle = { temp1[0], temp1[1], temp1[2] };
      auto & temp2 = pose.Keypoint( RIGHT_ANKLE );
      rAnkle = { temp2[0], temp2[1], temp2[2] };
   }
   else
   {
      throw std::runtime_error( "Ankle keypoints are required" );
   }
   
   // Create some useful vectors
   Eigen::Vector3d upVector( 0, 1, 0 );
   Eigen::Vector3d forwardVector( 0, 0, 1 );
   Eigen::Vector3d rHipVector = rKnee - rHip;
   Eigen::Vector3d lHipVector = lKnee - lHip;
   Eigen::Vector3d hipsVector = lHip - rHip;
   Eigen::Vector3d rKneeVector = rAnkle - rKnee;
   Eigen::Vector3d lKneeVector = lAnkle - lKnee;
   
   // Hip offsets
   Eigen::Vector3d pelvisToRHipVector = -hipsVector / 2;
   rig.rHip.offset = { pelvisToRHipVector[0], pelvisToRHipVector[1], pelvisToRHipVector[2] };
   Eigen::Vector3d pelvisToLHipVector = hipsVector / 2;
   rig.lHip.offset = { pelvisToLHipVector[0], pelvisToLHipVector[1], pelvisToLHipVector[2] };
   
   // Hips:
   // We assume a T-pose so, unlike the spine, hips point down instead of up.
   // This means for zero rotation of the hips:
   //   +Y points down along the femur
   //   +Z points the same as the spine in rest pose, so the forward vector is the same
   //   +X is pointing LEFT, opposite the spine due to +Y pointing down along the femur
   //
   //  1 Get a quaternion that adjusts for both T-pose and pelvis orienation:
   //    a) apply the pelvis rotation
   //    b) rotate the hips around the Z-axis so as to negate the T-pose
   //  2 Get a quaternion, per hip, from the rest pose to the actual pose
   //  3 Get the "bend" of the knee as an angle
   //  4 Calculate the roll of each hip vector, but only one of two ways. For each leg:
   //    a) If the bend of the knee is greater than our bend_threshold
   //         - Determine the forward vector by using the double-cross method betwen the hip-knee and hip-ankle vectors
   //       Else the bend of the knee is mostly straight
   //         - Determine the forward vector by using the double-cross method betwen the hip-knee and pelvisForward vectors
   //    b) Reorient the hip vector to rest pose where +Y is up. The Y component should be zero as this is a projection onto the XZ plane
   //    c) Calculate the roll as the rotation from the world forward vector to our current XZ vector.
   //    d) Also calculate and store the roll as a radian offset. The angle calculated will be directionless (always positive), but
   //       we know that we are projected on the XZ plane which allows us to simply multiply the angle with Y to get the direction.
   //  5 Apply the hip roll adjustment to the hip rotation, so that the knee points the correct way now
   //  6 Set the values for rotation and length
   
   // 1
   Eigen::Quaterniond restPoseAdjustmentRotatation = pelvisQuaternion * Eigen::AngleAxisd( M_PI, forwardVector );

   // 2
   Eigen::Quaterniond rHipRotation = Eigen::Quaterniond::FromTwoVectors( upVector, restPoseAdjustmentRotatation.inverse()._transformVector( rHipVector ) );
   Eigen::Quaterniond lHipRotation = Eigen::Quaterniond::FromTwoVectors( upVector, restPoseAdjustmentRotatation.inverse()._transformVector( lHipVector ) );
   
   // 3
   double rKneeBendAngle = std::acos( rHipVector.normalized().dot( rKneeVector.normalized() ) );
   double lKneeBendAngle = std::acos( lHipVector.normalized().dot( lKneeVector.normalized() ) );
   
   // 4
   Eigen::Quaterniond rHipRoll, lHipRoll;
   Eigen::Vector3d rHipForwardVector, lHipForwardVector;
   if ( rKneeBendAngle > BEND_THRESHOLD )
   {
      // 4a (right knee bent)
      Eigen::Vector3d rHipAnkleUnitVector = (rAnkle - rHip).normalized();
      rHipForwardVector = rHipVector.normalized().cross( rHipAnkleUnitVector ).cross( rHipVector.normalized() ).normalized();
   }
   else
   {
      // 4a (right knee straight)
      rHipForwardVector = rHipVector.normalized().cross( -hipsVector.normalized() ).normalized();
   }
   
   if ( lKneeBendAngle > BEND_THRESHOLD )
   {
      // 4a (left knee bent)
      Eigen::Vector3d lHipAnkleUnitVector = (lAnkle - lHip).normalized();
      lHipForwardVector = lHipVector.normalized().cross( lHipAnkleUnitVector ).cross( lHipVector.normalized() ).normalized();
   }
   else
   {
      // 4a (left knee straight)
      lHipForwardVector = lHipVector.normalized().cross( -hipsVector.normalized() ).normalized();
   }
   
   // 4b
   rHipForwardVector = (restPoseAdjustmentRotatation * rHipRotation).inverse()._transformVector( -rHipForwardVector );
   lHipForwardVector = (restPoseAdjustmentRotatation * lHipRotation).inverse()._transformVector( -lHipForwardVector );

   // 4c
   rHipRoll = Eigen::Quaterniond::FromTwoVectors( forwardVector, rHipForwardVector );
   lHipRoll = Eigen::Quaterniond::FromTwoVectors( forwardVector, lHipForwardVector );
   
   // 4d
   Eigen::AngleAxisd rHipRollAngle( rHipRoll );
   rig.rHip.roll = rHipRollAngle.angle() * rHipRollAngle.axis()[1];
   Eigen::AngleAxisd lHipRollAngle( lHipRoll );
   rig.lHip.roll = lHipRollAngle.angle() * lHipRollAngle.axis()[1];

   // 5
   rHipRotation = rHipRotation * rHipRoll;
   lHipRotation = lHipRotation * lHipRoll;
   
   // 6
   rig.rHip.quaternion = Utility::QuaternionToRaw( rHipRotation );
   rig.rHip.quaternionAbs = Utility::QuaternionToRaw( restPoseAdjustmentRotatation * rHipRotation );
   rig.lHip.quaternion = Utility::QuaternionToRaw( lHipRotation );
   rig.lHip.quaternionAbs = Utility::QuaternionToRaw( restPoseAdjustmentRotatation * lHipRotation );
   rig.rHip.length = rHipVector.norm();
   rig.lHip.length = lHipVector.norm();
   
   // Knees
   // More-or-less straightforward, just remember to include the rest pose adjustment
   Eigen::Quaterniond rKneeRotation = Eigen::Quaterniond::FromTwoVectors( upVector, (restPoseAdjustmentRotatation * rHipRotation).inverse()._transformVector( rKneeVector ) );
   Eigen::Quaterniond lKneeRotation = Eigen::Quaterniond::FromTwoVectors( upVector, (restPoseAdjustmentRotatation * lHipRotation).inverse()._transformVector( lKneeVector ) );
   rig.rKnee.quaternion = Utility::QuaternionToRaw( rKneeRotation );
   rig.rKnee.quaternionAbs = Utility::QuaternionToRaw( (restPoseAdjustmentRotatation * rHipRotation) * rKneeRotation );
   rig.lKnee.quaternion = Utility::QuaternionToRaw( lKneeRotation );
   rig.lKnee.quaternionAbs = Utility::QuaternionToRaw( (restPoseAdjustmentRotatation * lHipRotation) * lKneeRotation );
   rig.rKnee.length = rKneeVector.norm();
   rig.lKnee.length = lKneeVector.norm();
}
void KpToRigHelper::HandleArms( Pose & pose )
{
   Rig & rig = pose.RigPose().GetRig();
   
   Eigen::Quaterniond spine4Quaternion = Utility::RawToQuaternion( rig.spine4.quaternionAbs );
   Eigen::Vector3d baseNeck;
   Eigen::Vector3d rShoulder;
   Eigen::Vector3d rElbow;
   Eigen::Vector3d rWrist;
   Eigen::Vector3d lShoulder;
   Eigen::Vector3d lElbow;
   Eigen::Vector3d lWrist;
   
   if ( pose.HasKeypoint( BASE_NECK ) )
   {
      auto & temp = pose.Keypoint( BASE_NECK );
      baseNeck = { temp[0], temp[1], temp[2] };
   }
   else
   {
      // TODO: Use shoulders and anything else we have to "guess" this
   }
   
   if ( pose.HasKeypoint( LEFT_SHOULDER ) )
   {
      auto & temp1 = pose.Keypoint( LEFT_SHOULDER );
      lShoulder = { temp1[0], temp1[1], temp1[2] };
      auto & temp2 = pose.Keypoint( RIGHT_SHOULDER );
      rShoulder = { temp2[0], temp2[1], temp2[2] };
   }
   else
   {
      throw std::runtime_error( "Shoulder keypoints are required" );
   }
   
   if ( pose.HasKeypoint( LEFT_ELBOW ) )
   {
      auto & temp1 = pose.Keypoint( LEFT_ELBOW );
      lElbow = { temp1[0], temp1[1], temp1[2] };
      auto & temp2 = pose.Keypoint( RIGHT_ELBOW );
      rElbow = { temp2[0], temp2[1], temp2[2] };
   }
   else
   {
      throw std::runtime_error( "Elbow keypoints are required" );
   }
   
   if ( pose.HasKeypoint( LEFT_WRIST ) )
   {
      auto & temp1 = pose.Keypoint( LEFT_WRIST );
      lWrist = { temp1[0], temp1[1], temp1[2] };
      auto & temp2 = pose.Keypoint( RIGHT_WRIST );
      rWrist = { temp2[0], temp2[1], temp2[2] };
   }
   else
   {
      throw std::runtime_error( "Wrist keypoints are required" );
   }
   
   // Useful vectors
   Eigen::Vector3d upVector( 0, 1, 0 );
   Eigen::Vector3d forwardVector( 0, 0, 1 );
   Eigen::Vector3d rShoulderVector = rElbow - rShoulder;
   Eigen::Vector3d lShoulderVector = lElbow - lShoulder;
   
   // Shoulder offsets
   Eigen::Vector3d baseNeckToRShoulder = rShoulder - baseNeck;
   rig.rShoulder.offset = { baseNeckToRShoulder[0], baseNeckToRShoulder[1], baseNeckToRShoulder[2] };
   Eigen::Vector3d baseNeckToLShoulder = lShoulder - baseNeck;
   rig.lShoulder.offset = { baseNeckToLShoulder[0], baseNeckToLShoulder[1], baseNeckToLShoulder[2] };
   
   // Shoulders:
   // We assume a T-pose so, unlike the spine4, shoulders point out (left, right) instead of up.
   // This means for zero rotation of the shoulders:
   //   +Y points left or right along the humerus
   //   +Z points the same as the spine in rest pose, so the forward vector is the same
   //   +X points up for the right shoulder and down for the left shoulder
   //
   //  1 Get a quaternion that adjusts for both T-pose and spine4 orienation:
   //    a) apply the spine4 rotation
   //    b) rotate the shoulders around the Z-axis so as to negate the T-pose
   //  2 Get a quaternion, per shoulder, from the rest pose to the actual pose
   //  3 Get a quaternion, per shoulder, that rolls each shoulder so +Z is normal to the shoulder/wrist plane:
   //    a) Get the forward vector we want by using the double-cross method betwen the shoulder-elbow and shoulcer-wrist vectors
   //    b) Reorient the shoulder vector to rest pose where +Y is up. The Y component should be zero as this is a projection onto the XZ plane
   //    c) Calculate the roll as the rotation from the world forward vector to our current XZ vector
   //  4 Multiply steps 3*4 (in order)
   //  5 Set the values for rotation and length
   
   // 1
   Eigen::Quaterniond rRestPoseAdjustmentRotatation = spine4Quaternion * Eigen::AngleAxisd(  M_PI / 2, forwardVector );
   Eigen::Quaterniond lRestPoseAdjustmentRotatation = spine4Quaternion * Eigen::AngleAxisd( -M_PI / 2, forwardVector );

   // 2
   Eigen::Quaterniond rShoulderRotation = Eigen::Quaterniond::FromTwoVectors( upVector, rRestPoseAdjustmentRotatation.inverse()._transformVector( rShoulderVector ) );
   Eigen::Quaterniond lShoulderRotation = Eigen::Quaterniond::FromTwoVectors( upVector, lRestPoseAdjustmentRotatation.inverse()._transformVector( lShoulderVector ) );

   // 3a
   Eigen::Vector3d rShoulderWristUnitVector = (rWrist - rShoulder).normalized();
   Eigen::Vector3d lShoulderWristUnitVector = (lWrist - lShoulder).normalized();
   Eigen::Vector3d rShoulderForwardVector = rShoulderVector.normalized().cross( rShoulderWristUnitVector ).cross( rShoulderVector.normalized() ).normalized();
   Eigen::Vector3d lShoulderForwardVector = lShoulderVector.normalized().cross( lShoulderWristUnitVector ).cross( lShoulderVector.normalized() ).normalized();

   // 3b
   rShoulderForwardVector = (rRestPoseAdjustmentRotatation * rShoulderRotation).inverse()._transformVector( rShoulderForwardVector );
   lShoulderForwardVector = (lRestPoseAdjustmentRotatation * lShoulderRotation).inverse()._transformVector( lShoulderForwardVector );

   // 3c
   Eigen::Quaterniond rShoulderRoll = Eigen::Quaterniond::FromTwoVectors( forwardVector, rShoulderForwardVector );
   Eigen::Quaterniond lShoulderRoll = Eigen::Quaterniond::FromTwoVectors( forwardVector, lShoulderForwardVector );

   // 4
   rShoulderRotation = rShoulderRotation * rShoulderRoll;
   lShoulderRotation = lShoulderRotation * lShoulderRoll;

   // 5
   rig.rShoulder.quaternion = Utility::QuaternionToRaw( rShoulderRotation );
   rig.rShoulder.quaternionAbs = Utility::QuaternionToRaw( rRestPoseAdjustmentRotatation * rShoulderRotation );
   rig.lShoulder.quaternion = Utility::QuaternionToRaw( lShoulderRotation );
   rig.lShoulder.quaternionAbs = Utility::QuaternionToRaw( lRestPoseAdjustmentRotatation * lShoulderRotation );
   rig.rShoulder.length = rShoulderVector.norm();
   rig.lShoulder.length = lShoulderVector.norm();

   // Elbows
   // More-or-less straightforward, just remember to include the rest keypoints adjustment
   Eigen::Vector3d rElbowVector = rWrist - rElbow;
   Eigen::Vector3d lElbowVector = lWrist - lElbow;
   Eigen::Quaterniond rElbowRotation = Eigen::Quaterniond::FromTwoVectors( upVector, (rRestPoseAdjustmentRotatation * rShoulderRotation).inverse()._transformVector( rElbowVector ) );
   Eigen::Quaterniond lElbowRotation = Eigen::Quaterniond::FromTwoVectors( upVector, (lRestPoseAdjustmentRotatation * lShoulderRotation).inverse()._transformVector( lElbowVector ) );
   rig.rElbow.quaternion = Utility::QuaternionToRaw( rElbowRotation );
   rig.rElbow.quaternionAbs = Utility::QuaternionToRaw( (rRestPoseAdjustmentRotatation * rShoulderRotation) * rElbowRotation );
   rig.lElbow.quaternion = Utility::QuaternionToRaw( lElbowRotation );
   rig.lElbow.quaternionAbs = Utility::QuaternionToRaw( (lRestPoseAdjustmentRotatation * lShoulderRotation) * lElbowRotation );
   rig.rElbow.length = rElbowVector.norm();
   rig.lElbow.length = lElbowVector.norm();
}
Joint KpToRigHelper::CreateJoint( const Joint & parentJoint,
   Eigen::Vector3d boneVector )
{
   Joint returnValue;
   Eigen::Quaterniond parentRotationAbs = Utility::RawToQuaternion( parentJoint.quaternionAbs );
   
   Eigen::Quaterniond quat = Eigen::Quaterniond::FromTwoVectors( Eigen::Vector3d::UnitY(), parentRotationAbs.inverse()._transformVector( boneVector ) );
   returnValue.length = boneVector.norm();
   returnValue.quaternion = Utility::QuaternionToRaw( quat );
   returnValue.quaternionAbs = Utility::QuaternionToRaw( parentRotationAbs * quat );
   return returnValue;
}
