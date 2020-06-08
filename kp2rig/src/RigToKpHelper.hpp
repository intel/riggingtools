#ifndef RigToKpHelper_hpp
#define RigToKpHelper_hpp

class Pose;

class RigToKpHelper
{
public:
   static void HandleSpine( Pose & pose );
   static void HandleLegs( Pose & pose );
   static void HandleArms( Pose & pose );
};
#endif
