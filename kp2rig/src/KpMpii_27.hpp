#ifndef KpMpii_27_hpp
#define KpMpii_27_hpp

#include <vector>
#include <sstream>
#include "Pose.hpp"
#include "RigPose.hpp"

// Similar to Mpii_20 but with two additional keypoints for the tips of the feet
// and facial keypoints
struct Mpii_27
{
   std::array< double, 3 > rightAnkle;
   std::array< double, 3 > rightKnee;
   std::array< double, 3 > rightHip;
   std::array< double, 3 > leftHip;
   std::array< double, 3 > leftKnee;
   std::array< double, 3 > leftAnkle;
   std::array< double, 3 > pelvis;
   std::array< double, 3 > baseNeck;
   std::array< double, 3 > baseHead;
   std::array< double, 3 > topHead;
   std::array< double, 3 > rightWrist;
   std::array< double, 3 > rightElbow;
   std::array< double, 3 > rightShoulder;
   std::array< double, 3 > leftShoulder;
   std::array< double, 3 > leftElbow;
   std::array< double, 3 > leftWrist;
   std::array< double, 3 > leftBigToe;
   std::array< double, 3 > leftSmallToe;
   std::array< double, 3 > leftHeel;
   std::array< double, 3 > rightBigToe;
   std::array< double, 3 > rightSmallToe;   
   std::array< double, 3 > rightHeel;
   std::array< double, 3 > nose;
   std::array< double, 3 > leftEye;
   std::array< double, 3 > rightEye;
   std::array< double, 3 > leftEar;
   std::array< double, 3 > rightEar;
};

class KpMpii_27 : public Pose
{
public:
   KpMpii_27() = default;
   KpMpii_27( std::string kpType,
      const std::map< KEYPOINT_TYPE, int > & kpLayout );
   KpMpii_27( const KpMpii_27 & rhs );
   virtual ~KpMpii_27(){}
   
   virtual Pose * Clone() const { return new KpMpii_27( *this ); }
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
   virtual bool ValidateFeet() const;
   
private:
   Mpii_27 _keypoints;
   class RigPose _rigPose;
   bool _rigCreated = false;
   const int FRAME_DATA_SIZE = sizeof( _keypoints ) / sizeof( double );
   std::array< double, 3 > _coordinateSystem;
};

inline const RigPose & KpMpii_27::RigPose() const
{
   return _rigPose;
}
inline RigPose & KpMpii_27::RigPose()
{
   return _rigPose;
}
inline void KpMpii_27::Keypoint( std::array< double, 3 > value, int index )
{
   *((std::array< double, 3 > *)&_keypoints + index) = value;
   _rigCreated = false;
}
inline void KpMpii_27::Keypoint( std::array< double, 3 > value, KEYPOINT_TYPE type )
{
   try
   {
      *((std::array< double, 3 > *)&_keypoints + _kpLayout.at( type )) = value;
      _rigCreated = false;
   }
   catch ( std::out_of_range & )
   {
      std::stringstream ss;
      ss << "No keypoint found for '" << KpTypeToStr( type ) << "'. Check kpDescriptor.json";
      throw std::runtime_error( ss.str() );
   }
}
inline const std::array< double, 3 > & KpMpii_27::Keypoint( KEYPOINT_TYPE type ) const
{
   return *((std::array< double, 3 > *)&_keypoints + _kpLayout.at( type ) );
}
inline std::array< double, 3 > & KpMpii_27::Keypoint( KEYPOINT_TYPE type )
{
   return const_cast< std::array< double, 3 > &>(static_cast<const Pose &>(*this).Keypoint( type ) );
}
inline bool KpMpii_27::HasKeypoint( KEYPOINT_TYPE type ) const
{
   return _kpLayout.count( type ) == 1;
}
inline const std::array< double, 3 > & KpMpii_27::CoordinateSystem() const
{
   return _coordinateSystem;
}

#endif
