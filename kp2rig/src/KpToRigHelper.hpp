#ifndef KpToRigHelper_hpp
#define KpToRigHelper_hpp

class Pose;

class KpToRigHelper
{
public:
   static void HandleSpine( Pose & pose );
   static void HandleLegs( Pose & pose );
   static void HandleArms( Pose & pose );
};

#endif
