#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include <json.hpp>
#include "PoseFactory.hpp"
#include "Utility.hpp"
#include "KpImporterFactory.hpp"

// Built-in pose types
#include "KpMpii_16.hpp"
#include "KpMpii_20.hpp"
#include "KpMop_19.hpp"
#include "KpSolidObject.hpp"

// Static registry of all known pose types, and developers can add custom pose types by calling 'RegisterPose()'
static std::unordered_map< std::string, std::function< Pose *(std::string kpType, const std::map< KEYPOINT_TYPE, int > & kpLayout) > > g_poseCreatorMap =
{
   { "mpii",        [](std::string kpType, const std::map< KEYPOINT_TYPE, int > & kpLayout) { return new KpMpii_16( kpType, kpLayout ); } },
   { "mpii_20",     [](std::string kpType, const std::map< KEYPOINT_TYPE, int > & kpLayout) { return new KpMpii_20( kpType, kpLayout ); } },
   { "mop_19",      [](std::string kpType, const std::map< KEYPOINT_TYPE, int > & kpLayout) { return new KpMop_19( kpType, kpLayout ); } },
   { "solidObject", [](std::string kpType, const std::map< KEYPOINT_TYPE, int > & kpLayout) { return new KpSolidObject( kpType, kpLayout ); } }
};

std::unique_ptr< Pose > PoseFactory::Create( std::string kpType,
   std::map< KEYPOINT_TYPE, int > & kpLayout )
{
   std::unique_ptr< Pose > returnValue;
   
   // Find the pose creator from our registry
   auto creator = g_poseCreatorMap.find( kpType );
   if ( creator == g_poseCreatorMap.end() )
   {
      std::stringstream ss;
      ss << "No data importer or pose class for keypoint type '" << kpType << "'";
      throw std::runtime_error( ss.str() );
   }
         
   // Create a pose class for this type
   returnValue = std::unique_ptr< Pose >( (*creator).second( kpType,
      kpLayout ) );
      
   return returnValue;
}
std::unique_ptr<Pose> PoseFactory::FromRigPose( const RigPose & rigPose,
   std::string kpType,
   const std::map< KEYPOINT_TYPE, int > & kpLayout )
{
   auto creator = g_poseCreatorMap.find( kpType );
   if ( creator == g_poseCreatorMap.end() )
      throw std::runtime_error( "Uknown keypoint type" );

   // Create the new pose
   std::unique_ptr< Pose > returnValue( (*creator).second( kpType, kpLayout ) );

   // Have the new pose build itself from the rig-pose
   if ( returnValue )
      returnValue->FromRigPose( rigPose );  

   return returnValue;
}
void PoseFactory::RegisterPose( std::string kpType,
   std::function< Pose *(std::string, const std::map< KEYPOINT_TYPE, int > &) > poseCreator )
{
   // Register the new type
   g_poseCreatorMap[ kpType ] = poseCreator;
}
