#ifndef Pose_hpp
#define Pose_hpp

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include <vector>
#include "KpType.hpp"
#include "KpImporter.hpp"

class RigPose;
class PoseFactory;
template <typename T> class KpCommon;

// Base class for all Pose types
class Pose
{
   friend class PoseFactory;
   template <typename T> friend class KpCommon;
   typedef class RigPose RigPose_t;
   
public:
   // This generates a rigged pose from a Pose if not already created,
   // then returns the result; otherwise a cached result is returned.
   // This must interpolate joints as needed to conform to the standard rig structure defined in RigPose.hpp
   // This is not expected to perform filtering or temporal clean-up.
   // Assume output rig coordinate system is RIGHT-HANDED (Maya, Blender)
   virtual RigPose_t & GenerateRig() = 0;
   
   // There are situations where we need to go backwards: given a RigPose create
   // what the original Pose would look like
   virtual void FromRigPose( const RigPose_t & rigPose ) = 0;
   
   // Unlike Rig::ToArrays() which outputs multiple arrays representing a hierarchical rig, this outputs the
   // original input data as a vector.
   virtual void InputDataToArray( std::vector< double > & serializedList ) = 0;
   
   // How many doubles make up a frame of input data?
   virtual size_t InputDataSize() const = 0;
   
   // returns nullptr if GenerateRig() has not been called or failed
   virtual const RigPose_t & RigPose() const = 0;
   virtual RigPose_t & RigPose() = 0;
   
   // Each pose uses specific keypoints (mpii, coco, etc.), but they share a common category.
   // Valid category values include:
   //  "humanoid"
   //  "solid_object"
   virtual std::string Category() const = 0;
   
   // Useful for testing, this function verifies that the generated rig
   // is valid
   virtual bool ValidateRig() const = 0;
   
   // Prototype pattern
   virtual Pose * Clone() const = 0;
   
   // Keypoint getter/setter
   // Keypoints can be set in the order they _appear_ or by KEYPOINT_TYPE, but they are only read by KEYPOINT_TYPE.
   // Your implementation will need to use the KpImporter::KeypointLayout mapping in order to figure out which
   //   keypoint this is (this layout ultimately comes from the kpDescriptor.json file).
   // Keypoints accessed by keypoint are _not_ the same as the original index they were set by.
   virtual void Keypoint( std::array< double, 3 > value, int index ) = 0;
   virtual void Keypoint( std::array< double, 3 > value, KEYPOINT_TYPE type ) = 0;
   virtual const std::array< double, 3 > & Keypoint( KEYPOINT_TYPE type ) const = 0;
   virtual std::array< double, 3 > & Keypoint( KEYPOINT_TYPE type ) = 0;
   virtual bool HasKeypoint( KEYPOINT_TYPE type ) const = 0;
   
   // Defines how the rig will define "up", "right", and "out".
   // Default is a right-handed coordinate system (1.0, 1.0, 1.0),
   // a left-handed coordinate system would look like (1.0, 1.0, -1.0).
   virtual void CoordinateSystem( std::array< double, 3 > value ) = 0;
   virtual const std::array< double, 3 > & CoordinateSystem() const = 0;
   
   // These are defined for you
   Pose() = default;
   Pose( const Pose & rhs )
      : _name( rhs._name ),
      _kpType( rhs._kpType ),
      _timestamp( rhs._timestamp ),
      _kpLayout( rhs._kpLayout ) {}
   Pose( std::string kpType,
      const std::map< KEYPOINT_TYPE, int > & kpLayout )
      : _kpType( kpType ),
      _kpLayout( kpLayout ){}
   virtual ~Pose(){}
   std::string Name() const { return _name; }
   void Name( std::string value ) { _name = value; }
   int Timestamp() const { return _timestamp; }
   void Timestamp( int value ) { _timestamp = value; }
   std::string KpType() const { return _kpType; }
   const std::map< KEYPOINT_TYPE, int > & KeypointLayout() const { return _kpLayout; }

protected:
   std::string _name;
   std::string _kpType;
   int _timestamp = 0;
   std::map< KEYPOINT_TYPE, int > _kpLayout;
};

#endif
