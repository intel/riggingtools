#ifndef RigPose_hpp
#define RigPose_hpp

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include <vector>
#include "Rig.hpp"

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
   
   // ratio == 0 is same as this, ratio == 1 is same as rhs
   RigPose Interpolate( const RigPose & rhs, double ratio ) const;
   
protected:
   void UpdateAbsRotations();
   
   int _timestamp = -1;
   std::string _kpType;
   Rig _rig;
};

#endif
