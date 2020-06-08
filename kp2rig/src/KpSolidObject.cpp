#define _USE_MATH_DEFINES
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "KpSolidObject.hpp"
#include "KpHelper.hpp"
#include "Utility.hpp"

KpSolidObject::KpSolidObject( std::string kpType,
   const std::map< KEYPOINT_TYPE, int > & kpLayout )
   : Pose( kpType, kpLayout )
{
}
KpSolidObject::KpSolidObject( const KpSolidObject & rhs )
   : Pose( rhs )
{
   _location = rhs._location;
   _rigPose = rhs._rigPose;
   _rigCreated = rhs._rigCreated;
   _coordinateSystem = rhs._coordinateSystem;
}
RigPose & KpSolidObject::GenerateRig()
{
   _rigPose.Timestamp( Timestamp() );
   Rig & rig = _rigPose.GetRig();
   
   rig.location = _location;
   rig.GetJoint( Rig::PELVIS ).quaternion = { 0,0,0,1 };
   rig.GetJoint( Rig::PELVIS ).quaternionAbs = { 0,0,0,1 };
   
   // Ensure we mark ONLY the data we intend to use, to avoid issues downstream
   rig.numJointOffsetsUsed = 0;
   rig.numLengthsUsed = 0;
   rig.numJointsUsed = 1;   
   
   _rigPose.KpType( KpType() );
   _rigCreated = true;
   return _rigPose;
}
void KpSolidObject::FromRigPose( const class RigPose & rigPose )
{
   _rigPose = rigPose;   
   Rig & rig = _rigPose.GetRig();
   
   _location = rig.location;
   
   _timestamp = rigPose.Timestamp();
   _rigCreated = true;
}
void KpSolidObject::InputDataToArray( std::vector< double > & serializedList )
{
   serializedList.push_back( _location[0] );
   serializedList.push_back( _location[1] );
   serializedList.push_back( _location[2] );
}
bool KpSolidObject::ValidateRig() const
{
   const double tolerance = 0.25;
   double distance;
   
   Eigen::Vector3d rigPelvis = Utility::RawToVector( _rigPose.GetRig().location );
   Eigen::Vector3d kpPelvis = Utility::RawToVector( _location );
   distance = (rigPelvis - kpPelvis).norm();
   if ( distance > tolerance )
      return false;
   
   return true;
}
void KpSolidObject::CoordinateSystem( std::array< double, 3 > value )
{
   _coordinateSystem = value;
   
   // x
   _location[ 0 ] *= _coordinateSystem[ 0 ];

   // y
   _location[ 1 ] *= _coordinateSystem[ 1 ];
   
   // z
   _location[ 2 ] *= _coordinateSystem[ 2 ];
}
