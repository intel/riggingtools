#ifndef AnimatedRig_hpp
#define AnimatedRig_hpp

#include <stdio.h>
#include <map>
#include "Pose.hpp"
#include <json.hpp>
#include "SmoothFactory.hpp"

class AnimatedRig
{
public:
   AnimatedRig();
   
   void AddPose( std::unique_ptr< Pose > & pose );
   std::map< int, std::unique_ptr< Pose > > & GetFrames() { return _frames; }
   const std::map< int, std::unique_ptr< Pose > > & GetFrames() const { return _frames; }
   void FixMissingFrames( int rangeStart,
      int rangeEnd,
      int missingFramesThreshold );
   std::string Category() const { return _category; }

   // Smooth the XYZ input data (NOT the output rig).
   // This means you will need to re-generate rigs after calling this if you want filtered/smoothed data.
   // Many implementations have inherent delays, so expect time shifts!
   // Range is all inclusive, [rangeStart, rangeEnd].
   // Range is updated with shifted timestamps on successful return.
   void SmoothFrames( Smooth::SMOOTH_TYPE type,
      int & rangeStart,
      int & rangeEnd,
      bool flush = false );

   int Write( nlohmann::json & json,
      int startTimestamp,
      int endTimestamp );
   
private:
   void SmoothAllKeypoints( Smooth::SMOOTH_TYPE type,
      int & rangeStart,
      int & rangeEnd,
      bool flush = false );
   void SmoothAllBoneRolls( Smooth::SMOOTH_TYPE type,
      int rangeStart,
      int rangeEnd,
      bool flush = false );
   void DetermineBoneLengths( std::map< int, std::unique_ptr< Pose > >::iterator & poseIt );
   void UpdateBoneLengths( std::unique_ptr< Pose > & pose );

   std::map< int, std::unique_ptr< Pose > > _frames;
   std::vector< std::vector< double > > _rawBoneLengths;
   std::vector< double > _averagedBoneLengths;
   std::vector< std::unique_ptr< Smooth > > _jointSmoothers;
   std::vector< std::unique_ptr< Smooth > > _boneRollSmoothers;
   std::array< std::unique_ptr< Smooth >, 3 > _locationSmoothers;
   std::string _category;
};

#endif
