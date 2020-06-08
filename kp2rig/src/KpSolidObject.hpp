#ifndef KpSolidObject_hpp
#define KpSolidObject_hpp

#include <vector>
#include "Pose.hpp"
#include "RigPose.hpp"

class KpSolidObject : public Pose
{
public:
   KpSolidObject() = default;
   KpSolidObject( std::string kpType,
      const std::map< KEYPOINT_TYPE, int > & kpLayout );
   KpSolidObject( const KpSolidObject & rhs );
   virtual ~KpSolidObject(){}
   
   virtual Pose * Clone() const { return new KpSolidObject( *this ); }
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
   virtual std::string Category() const { return "solidObject"; }
   virtual bool ValidateRig() const;
   virtual void CoordinateSystem( std::array< double, 3 > value );
   virtual const std::array< double, 3 > & CoordinateSystem() const;
   
private: 
   std::array< double, 3 > _location;
   class RigPose _rigPose;
   bool _rigCreated = false;
   const int FRAME_DATA_SIZE = 3;
   std::array< double, 3 > _coordinateSystem;
};

inline const RigPose & KpSolidObject::RigPose() const
{
   return _rigPose;
}
inline RigPose & KpSolidObject::RigPose()
{
   return _rigPose;
}
inline void KpSolidObject::Keypoint( std::array< double, 3 > value, int index )
{
   _location = value;
   (void)index; // Avoids a compiler warning
}
inline void KpSolidObject::Keypoint( std::array< double, 3 > value, KEYPOINT_TYPE type )
{
   _location = value;
   (void)type; // Avoids a compiler warning
}
inline const std::array< double, 3 > & KpSolidObject::Keypoint( KEYPOINT_TYPE type ) const
{
   return _location;
   (void)type; // Avoids a compiler warning
}
inline std::array< double, 3 > & KpSolidObject::Keypoint( KEYPOINT_TYPE type )
{
   return const_cast< std::array< double, 3 > &>(static_cast<const Pose &>(*this).Keypoint( type ) );
}
inline bool KpSolidObject::HasKeypoint( KEYPOINT_TYPE type ) const
{
   return _kpLayout.count( type ) == 1;
}
inline const std::array< double, 3 > & KpSolidObject::CoordinateSystem() const
{
   return _coordinateSystem;
}

#endif
