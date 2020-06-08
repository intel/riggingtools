#ifndef PoseFactory_hpp
#define PoseFactory_hpp

#include <string>
#include <memory>
#include <stdio.h>
#include <functional>
#include "Pose.hpp"

class RigPose;

class PoseFactory
{
public:
   static std::unique_ptr< Pose > Create( std::string kpType,
      std::map< KEYPOINT_TYPE, int > & kpLayout );
   static std::unique_ptr< Pose > FromRigPose( const RigPose & rigPose,
      std::string kpType,
      const std::map< KEYPOINT_TYPE, int > & kpLayout );
   static void RegisterPose( std::string kpType,
      std::function< Pose *(std::string, const std::map< KEYPOINT_TYPE, int > &) > poseCreator );
};

#endif
