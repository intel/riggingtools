#ifndef KpMop_19_hpp
#define KpMop_19_hpp

#include <vector>
#include "Pose.hpp"
#include "RigPose.hpp"

// Modified OpenPose, notice the difference in layout.
struct Mop_19
{
   std::array< double, 3 > topHead;
   std::array< double, 3 > baseNeck;
   std::array< double, 3 > rightShoulder;
   std::array< double, 3 > rightElbow;
   std::array< double, 3 > rightWrist;
   std::array< double, 3 > leftShoulder;
   std::array< double, 3 > leftElbow;
   std::array< double, 3 > leftWrist;
   std::array< double, 3 > rightHip;
   std::array< double, 3 > rightKnee;
   std::array< double, 3 > rightAnkle;
   std::array< double, 3 > leftHip;
   std::array< double, 3 > leftKnee;
   std::array< double, 3 > leftAnkle;
   std::array< double, 3 > rightEar;
   std::array< double, 3 > rightEye;
   std::array< double, 3 > nose;
   std::array< double, 3 > leftEye;
   std::array< double, 3 > leftEar;
};


class KpMop_19 : public Pose
{
public:
   KpMop_19() = default;
   KpMop_19( std::string kpType,
      const std::map< KEYPOINT_TYPE, int > & kpLayout );
   KpMop_19( const KpMop_19 & rhs );
   virtual ~KpMop_19(){}
   
   virtual Pose * Clone() const { return new KpMop_19( *this ); }
   virtual void Keypoint( std::array< double, 3 > value, int index );
   virtual void Keypoint( std::array< double, 3 > value, KEYPOINT_TYPE type );
   virtual const std::array< double, 3 > & Keypoint( KEYPOINT_TYPE type ) const;
   virtual std::array< double, 3 > & Keypoint( KEYPOINT_TYPE type );
   virtual bool HasKeypoint( KEYPOINT_TYPE type ) const;
   virtual class RigPose & GenerateRig();
   virtual void FromRigPose( const class RigPose & rigPose );
   virtual void InputDataToArray( std::vector< double > & serializedList );
   virtual size_t InputDataSize() const { return FRAME_DATA_SIZE; }
   virtual const class RigPose & RigPose() const;
   virtual class RigPose & RigPose();
   virtual std::string Category() const { return "humanoid"; }
   virtual bool ValidateRig() const;
   virtual void CoordinateSystem( std::array< double, 3 > value );
   virtual const std::array< double, 3 > & CoordinateSystem() const;
   
protected:
   virtual void HandleHands();
   virtual void HandleFeet();
   
private:
   Mop_19 _keypoints;
   
   class RigPose _rigPose;
   bool _rigCreated = false;
   const int FRAME_DATA_SIZE = sizeof( _keypoints ) / sizeof( double );
   std::array< double, 3 > _coordinateSystem;
};

inline const RigPose & KpMop_19::RigPose() const
{
   return _rigPose;
}
inline RigPose & KpMop_19::RigPose()
{
   return _rigPose;
}
inline void KpMop_19::Keypoint( std::array< double, 3 > value, int index )
{
   *((std::array< double, 3 > *)&_keypoints + index) = value;
   _rigCreated = false;
}
inline std::array< double, 3 > & KpMop_19::Keypoint( KEYPOINT_TYPE type )
{
   return const_cast< std::array< double, 3 > &>(static_cast<const Pose &>(*this).Keypoint( type ) );
}
inline bool KpMop_19::HasKeypoint( KEYPOINT_TYPE type ) const
{
   return _kpLayout.count( type ) == 1;
}
inline const std::array< double, 3 > & KpMop_19::CoordinateSystem() const
{
   return _coordinateSystem;
}

#endif
