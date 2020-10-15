#ifndef RigPose_hpp
#define RigPose_hpp

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include <map>
#include "Rig.hpp"

// Supplimentary joints are keypoints of importance not included in the final rig.
struct SupplimentaryJoint : public Joint
{
   SupplimentaryJoint() = default;
   SupplimentaryJoint( std::string name,
      std::string parentName,
      const Joint & joint )
      : Joint( joint ), name{ name }, parentName{ parentName } {}
   SupplimentaryJoint( const SupplimentaryJoint & ) = default;
   std::string name;
   std::string parentName;
};

// Wrapper for a rig, has extra information not contained in the rig
class RigPose
{
public:
   RigPose() = default;
   RigPose( int timestamp, const Rig & rig );
   RigPose( const RigPose & ) = default;
   RigPose( RigPose && ) = default;
   RigPose & operator=( const RigPose & ) = default;

   std::string KpType() const { return _kpType; }
   void KpType( std::string v ) { _kpType = v; }
   int Timestamp() const { return _timestamp; }
   void Timestamp( int value ) { _timestamp = value; }
   
   Rig & GetRig() { return _rig; }
   const Rig & GetRig() const { return _rig; }
   
   // @ratio is a value in the range [0,1], where @ratio == 0 is same as this, @ratio == 1 is same as rhs
   RigPose Interpolate( const RigPose & rhs, double ratio ) const;
   
   // Supplimentary joints are keypoints of importance not included in the final rig.
   std::map< std::string, SupplimentaryJoint > SupplimentaryJoints;
   
protected:
   void UpdateAbsRotations();
   
   int _timestamp = -1;
   std::string _kpType;
   Rig _rig;
};

#endif
