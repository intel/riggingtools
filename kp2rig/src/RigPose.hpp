#ifndef RigPose_hpp
#define RigPose_hpp

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include <vector>
#include "Rig.hpp"

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
   
   // Supplimentary joints as {parent name, joint} pairs.
   // These are keypoints of importance not included in the final rig.
   // If there is a hierarchy to the supplimentary joints, ensure they are added
   // in hierarchical order, with the parent coming first.
   std::vector< std::pair< std::string, Joint > > _supplimentaryJoints;
   
protected:
   void UpdateAbsRotations();
   
   int _timestamp = -1;
   std::string _kpType;
   Rig _rig;
};

#endif
