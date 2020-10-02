#ifndef Rig_hpp
#define Rig_hpp

#include <array>
#include <unordered_map>
#include <mutex>

// Global strings
#define PELVIS_STRING    "pelvis"
#define RHIP_STRING      "rHip"
#define RKNEE_STRING     "rKnee"
#define RANKLE_STRING    "rAnkle"
#define RTOEBASE_STRING  "rToeBase"
#define LHIP_STRING      "lHip"
#define LKNEE_STRING     "lKnee"
#define LANKLE_STRING    "lAnkle"
#define LTOEBASE_STRING  "lToeBase"
#define SPINE2_STRING    "spine2"
#define SPINE3_STRING    "spine3"
#define SPINE4_STRING    "spine4"
#define RSHOULDER_STRING "rShoulder"
#define RELBOW_STRING    "rElbow"
#define RWRIST_STRING    "rWrist"
#define LSHOULDER_STRING "lShoulder"
#define LELBOW_STRING    "lElbow"
#define LWRIST_STRING    "lWrist"
#define BASENECK_STRING  "baseNeck"
#define BASEHEAD_STRING  "baseHead"

// This represents a joint, including the root joint and all child joints.
struct Joint
{
   // This is relative to the rest (bind) pose + parent rotation, NOT world rotation.
   // This is the value you would expect to see in animation software.
   std::array< double, 4 > quaternion = { 0,0,0,1 };
   
   // This IS world rotation, independent of parent and rest (bind) pose
   std::array< double, 4 > quaternionAbs = { 0,0,0,1 };
   
   // This is the length of the limb, starting at this joint and pointing to its child.
   // This is the radius for single-joint objects.
   double length = 0.;
   
   // This represents the bone roll, in radians, about a limb's local Y axis (so the "twist" of a limb) and
   // is primarly useful for joints with more than one degree of freedom (hips and shoulders, for example).
   // This is already baked into both quaternions above but is kept separate to help smooth noisy roll data.
   // Not all joints need this so it's okay to leave at zero.
   double roll = 0.;
   
   // Some joints, such as hips and shoulders, have offsets from their parents.
   // This is to be interpreted as an offset vector relative to the parent.
   std::array< double, 3 > offset = { 0 };
};

typedef std::array< double, 3 > JointOffset;

// This defines our standard output rig
struct Rig
{
   Rig(){}
   ~Rig(){}
   
   static const size_t MAX_NUM_JOINTS = 20;
   enum JOINT_TYPE
   {
      PELVIS = 0,
      RHIP,
      RKNEE,
      RANKLE,
      RTOEBASE,
      LHIP,
      LKNEE,
      LANKLE,
      LTOEBASE,
      SPINE2,
      SPINE3,
      SPINE4,
      RSHOULDER,
      RELBOW,
      RWRIST,
      LSHOULDER,
      LELBOW,
      LWRIST,
      BASENECK,
      BASEHEAD,
      JOINT_TYPE_UNKNOWN
   };
   int numJointsUsed = MAX_NUM_JOINTS;
   
   static const size_t MAX_NUM_JOINT_OFFSETS = 4;
   enum JOINT_OFFSET_TYPE
   {
      PELVIS_TO_RHIP = 0,
      PELVIS_TO_LHIP,
      BASE_NECK_TO_RSHOULDER,
      BASE_NECK_TO_LSHOULDER
   };
   int numJointOffsetsUsed = MAX_NUM_JOINT_OFFSETS;
   
   // Position in world coordinates of the pelvis
   static const size_t LOCATION_DIMENSION = 3;
   std::array< double, 3 > location = {0.0};
   
   static const size_t MAX_LENGTHS_DIMENSION = MAX_NUM_JOINTS;
   int numLengthsUsed = MAX_NUM_JOINTS;
   
   static const size_t MAX_ROTATIONS_DIMENSION = MAX_NUM_JOINTS * 4;
   static const size_t MAX_OFFSETS_DIMENSION = MAX_NUM_JOINT_OFFSETS * 3;
   
   // How our rig is defined.
   // The pelvis points up, consider this the same as the lowest spine joint
   union { Joint pelvis; Joint spine1; };
      Joint rHip;
         Joint rKnee;
            Joint rAnkle;
               Joint rToeBase;
      Joint lHip;
         Joint lKnee;
            Joint lAnkle;
               Joint lToeBase;
      Joint spine2;
         Joint spine3;
            Joint spine4;
               Joint rShoulder;
                  Joint rElbow;
                     Joint rWrist;
               Joint lShoulder;
                  Joint lElbow;
                     Joint lWrist;
               Joint baseNeck;
                  Joint baseHead;  // Tail of this is the top of the head

   void ToArrays( std::vector< double > & locations,
      std::vector< double > & lengths,
      std::vector< double > & rotations,
      std::vector< double > & offsets ) const
   {
      // Position of the root
      for ( int i=0; i<3; ++i ) locations.push_back( location[i] );
      
      // All joint rotations
      for ( int i = 0; i < numJointsUsed; ++i )
      {
         const Joint & joint = GetJoint( JOINT_TYPE(i) );
         for ( int j=0; j<4; ++j ) rotations.push_back( joint.quaternion[j] );
      }
      
      // All joint lengths
      for ( int i = 0; i < numLengthsUsed; ++i )
      {
         const Joint & joint = GetJoint( JOINT_TYPE(i) );
         lengths.push_back( joint.length );
      }
      
      // All joint offsets
      for ( int i = 0; i < numJointOffsetsUsed; ++i )
      {
         const JointOffset & jointOffset = GetJointOffset( JOINT_OFFSET_TYPE(i) );
         for ( int j=0; j<3; ++j ) offsets.push_back( jointOffset[j] );
      }
   }
   static JOINT_TYPE GetJointType( std::string type )
   {
      static std::unordered_map< std::string, JOINT_TYPE > map = {
         { PELVIS_STRING,    PELVIS },
         { RHIP_STRING,      RHIP },
         { RKNEE_STRING,     RKNEE },
         { RANKLE_STRING,    RANKLE },
         { RTOEBASE_STRING,  RTOEBASE },
         { LHIP_STRING,      LHIP },
         { LKNEE_STRING,     LKNEE },
         { LANKLE_STRING,    LANKLE },
         { LTOEBASE_STRING,  LTOEBASE },
         { SPINE2_STRING,    SPINE2 },
         { SPINE3_STRING,    SPINE3 },
         { SPINE4_STRING,    SPINE4 },
         { RSHOULDER_STRING, RSHOULDER },
         { RELBOW_STRING,    RELBOW },
         { RWRIST_STRING,    RWRIST },
         { LSHOULDER_STRING, LSHOULDER },
         { LELBOW_STRING,    LELBOW },
         { LWRIST_STRING,    LWRIST },
         { BASENECK_STRING,  BASENECK },
         { BASEHEAD_STRING,  BASEHEAD }
      };
      
      auto it = map.find( type );
      if ( it == map.end() )
         return PELVIS;
      else
         return (*it).second;
   }
   // I intentionally made the return 'const char *' instead of 'std::string', because
   // returning a static character array means it can be passed directly to a C function,
   // which is helpful for the C API
   static const char * GetJointType( JOINT_TYPE type )
   {
      switch ( type )
      {
         case PELVIS:   return PELVIS_STRING;
         case RHIP:     return RHIP_STRING;
         case RKNEE:    return RKNEE_STRING;
         case RANKLE:   return RANKLE_STRING;
         case RTOEBASE: return RTOEBASE_STRING;
         case LHIP:     return LHIP_STRING;
         case LKNEE:    return LKNEE_STRING;
         case LANKLE:   return LANKLE_STRING;
         case LTOEBASE: return LTOEBASE_STRING;
         case SPINE2:   return SPINE2_STRING;
         case SPINE3:   return SPINE3_STRING;
         case SPINE4:   return SPINE4_STRING;
         case RSHOULDER:return RSHOULDER_STRING;
         case RELBOW:   return RELBOW_STRING;
         case RWRIST:   return RWRIST_STRING;
         case LSHOULDER:return LSHOULDER_STRING;
         case LELBOW:   return LELBOW_STRING;
         case LWRIST:   return LWRIST_STRING;
         case BASENECK: return BASENECK_STRING;
         case BASEHEAD: return BASEHEAD_STRING;
         default: return "";
      };
   }
   Joint & GetJoint( std::string type )
   {
         return GetJoint( GetJointType( type ) );
   }
   const Joint & GetJoint( std::string type ) const { return const_cast< Rig * >(this)->GetJoint( type ); }
   Joint & GetJoint( JOINT_TYPE type )
   {
      switch ( type )
      {
         case PELVIS:    return pelvis;
         case RHIP:      return rHip;
         case RKNEE:     return rKnee;
         case RANKLE:    return rAnkle;
         case RTOEBASE:  return rToeBase;
         case LHIP:      return lHip;
         case LKNEE:     return lKnee;
         case LANKLE:    return lAnkle;
         case LTOEBASE:  return lToeBase;
         case SPINE2:    return spine2;
         case SPINE3:    return spine3;
         case SPINE4:    return spine4;
         case RSHOULDER: return rShoulder;
         case RELBOW:    return rElbow;
         case RWRIST:    return rWrist;
         case LSHOULDER: return lShoulder;
         case LELBOW:    return lElbow;
         case LWRIST:    return lWrist;
         case BASENECK:  return baseNeck;
         case BASEHEAD:  return baseHead;
         default:        return pelvis;
      }
   }
   const Joint & GetJoint( JOINT_TYPE type ) const { return const_cast< Rig * >(this)->GetJoint( type ); }
   JointOffset GetJointOffset( JOINT_OFFSET_TYPE type )
   {
      switch ( type )
      {
         case PELVIS_TO_RHIP:         return rHip.offset;
         case PELVIS_TO_LHIP:         return lHip.offset;
         case BASE_NECK_TO_RSHOULDER: return rShoulder.offset;
         case BASE_NECK_TO_LSHOULDER: return lShoulder.offset;
         default:                     return { 0, 0, 0 };
      }
   }
   const JointOffset GetJointOffset( JOINT_OFFSET_TYPE type ) const { return const_cast< Rig * >(this)->GetJointOffset( type ); }
   
   static JOINT_TYPE GetJointParent( JOINT_TYPE type )
   {
      switch ( type )
      {
         case PELVIS:    return JOINT_TYPE_UNKNOWN;
         case RHIP:      return PELVIS;
         case RKNEE:     return RHIP;
         case RANKLE:    return RKNEE;
         case RTOEBASE:  return RANKLE;
         case LHIP:      return PELVIS;
         case LKNEE:     return LHIP;
         case LANKLE:    return LKNEE;
         case LTOEBASE:  return LANKLE;
         case SPINE2:    return PELVIS;
         case SPINE3:    return SPINE2;
         case SPINE4:    return SPINE3;
         case RSHOULDER: return SPINE4;
         case RELBOW:    return RSHOULDER;
         case RWRIST:    return RELBOW;
         case LSHOULDER: return SPINE4;
         case LELBOW:    return LSHOULDER;
         case LWRIST:    return LELBOW;
         case BASENECK:  return SPINE4;
         case BASEHEAD:  return BASENECK;
         default:        return JOINT_TYPE_UNKNOWN;
      }
   }
   // This creates a default armature, or rest (bind) pose, that is needed in order to make sense of rig data.
   // This uses the Singleton pattern.
   // The values in the returned rig will provide:
   //  - Default rotations
   //  - Default joint (really bone) lengths
   //  - Default joint offsets
   static Rig DefaultPoseHumanoid()
   {
      static Rig instance;
      static bool initialized = false;
      if ( !initialized )
      {
         // Make this singleton thread-safe
         static std::mutex m;
         std::lock_guard< std::mutex > autoLock( m );
         if ( initialized )
            return instance;
         initialized = true;
         
         // Lengths were determined by averaging American football players over a few hundred frames,
         // then multiplying everything by 0.9 since an average person is smaller.

         //--------------- JOINTS --------------------
         // Pelvis joint, the parent of all joints
         instance.pelvis.quaternion = { 0, 0, 0, 1 };
         instance.pelvis.length = 0.126220303905998;

         // rHip rotated about the Z axis PI
         instance.rHip.quaternion = { 0, 0, 1, 0 };
         instance.rHip.length = 0.370963097762336;

         // rKnee
         instance.rKnee.quaternion = { 0, 0, 0, 1 };
         instance.rKnee.length = 0.386561431929226;

         // rAnkle rotated about the X axis PI*7/18
         instance.rAnkle.quaternion = { 0.5735764, 0, 0, 0.819152 };
         instance.rAnkle.length = 0.141880341123086;

         // rToeBase rotated about the X axis PI/9
         instance.rToeBase.quaternion = { 0.1736482, 0, 0, 0.9848078 };
         instance.rToeBase.length = 0.04729344704103;

         // lHip
         instance.lHip.quaternion = instance.rHip.quaternion;
         instance.lHip.length = instance.rHip.length;

         // lKnee
         instance.lKnee.quaternion = instance.rKnee.quaternion;
         instance.lKnee.length = instance.rKnee.length;

         // lAnkle
         instance.lAnkle.quaternion = instance.rAnkle.quaternion;
         instance.lAnkle.length = instance.rAnkle.length;

         // lToeBase
         instance.lToeBase.quaternion = instance.rToeBase.quaternion;
         instance.lToeBase.length = instance.rToeBase.length;

         // spine2
         instance.spine2.quaternion = { 0, 0, 0, 1 };
         instance.spine2.length = 0.122532125318093;

         // spine3
         instance.spine3.quaternion = { 0, 0, 0, 1 };
         instance.spine3.length = 0.126637363035725;

         // spine4
         instance.spine4.quaternion = { 0, 0, 0, 1 };
         instance.spine4.length = 0.122532125318093;

         // rShoulder rotated about the Z axis PI/2
         instance.rShoulder.quaternion = { 0, 0, 0.7071068, 0.7071068 };
         instance.rShoulder.length = 0.266648027895734;

         // rElbow
         instance.rElbow.quaternion = { 0, 0, 0, 1 };
         instance.rElbow.length = 0.207658895203634;

         // rWrist
         instance.rWrist.quaternion = { 0, 0, 0, 1 };
         instance.rWrist.length = 0.132146569675043;

         // lShoulder rotated about the Z axis -PI/2
         instance.lShoulder.quaternion = { 0, 0, -0.7071068, 0.7071068 };
         instance.lShoulder.length = instance.rShoulder.length;

         // lElbow
         instance.lElbow.quaternion = instance.rElbow.quaternion;
         instance.lElbow.length = instance.rElbow.length;

         // lWrist
         instance.lWrist.quaternion = instance.rWrist.quaternion;
         instance.lWrist.length = instance.rWrist.length;

         // Base of neck (torso-ish)
         instance.baseNeck.quaternion = { 0, 0, 0, 1 };
         instance.baseNeck.length = 0.084940724767463;

         // Base of head (not quite chin but close)
         instance.baseHead.quaternion = { 0, 0, 0, 1 };
         instance.baseHead.length = 0.204343368655985;

         //--------------- OFFSETS --------------------
         instance.rHip.offset = { -0.096862528167303, 0, 0 };
         instance.lHip.offset = {  0.096862528167303, 0, 0 };
         instance.rShoulder.offset = { -0.169329877973758, 0, 0 };
         instance.lShoulder.offset = {  0.169329877973758, 0, 0 };
      }
      
      return instance;
   }
   static constexpr auto RestPoseHumanoid = DefaultPoseHumanoid;
};
#endif 
