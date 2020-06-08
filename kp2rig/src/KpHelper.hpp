#ifndef KpHelper_hpp
#define KpHelper_hpp

class Pose;

class KpHelper
{
public:
   static bool ValidateSpine( const Pose & pose );
   static bool ValidateLegs( const Pose & pose );
   static bool ValidateArms( const Pose & pose );
   static void KeypointsRhToLh( Pose & pose );
};

#endif
