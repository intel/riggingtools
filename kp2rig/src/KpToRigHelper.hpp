#ifndef KpToRigHelper_hpp
#define KpToRigHelper_hpp

#include "Utility.hpp"
#include "Rig.hpp"

class Pose;

class KpToRigHelper
{
public:
   static void HandleSpine( Pose & pose );
   static void HandleLegs( Pose & pose );
   static void HandleArms( Pose & pose );
   
   static Joint CreateJoint( const Joint & parentJoint,
      Eigen::Vector3d boneVector );
};

#endif
