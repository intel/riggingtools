#ifndef RigToKpHelper_hpp
#define RigToKpHelper_hpp

#include <array>
#include "Rig.hpp"

class Pose;

class RigToKpHelper
{
public:
   static void HandleSpine( Pose & pose );
   static void HandleLegs( Pose & pose );
   static void HandleArms( Pose & pose );
   
   static std::array< double, 3 > WorldCoordinates( const Rig & rig,
      Rig::JOINT_TYPE type );
};
#endif
