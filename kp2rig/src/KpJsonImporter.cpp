#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string.h>
#include "Utility.hpp"
#include "PoseFactory.hpp"
#include "KpImporterFactory.hpp"
#include "KpJsonImporter.hpp"

void KpJsonImporter::Open( std::string filename )
{
   // Open the file
   std::ifstream i( Utility::ExpandTilde( filename ) );
   if ( !i.good() )
   {
      std::stringstream ss;
      char buffer[512];
      buffer[0] = 0;
      auto dontCare = strerror_s( buffer, sizeof( buffer ), errno ); (void)dontCare;
      ss << "Could not open '" << filename << "': " << buffer;
      throw std::runtime_error( ss.str() );
   }
   
   // Parse the JSON file using overloaded istream-like operator
   try
   {
      i >> _json;
   }
   // For when we move to v3: catch ( nlohmann::json::parse_error e )
   catch ( std::invalid_argument & e )
   {
      std::stringstream ss;
      ss << "Could not parse '" << filename << "': " << e.what();
      throw std::runtime_error( ss.str() );
   }
   catch ( ... )
   {
      throw std::runtime_error( "Unknown JSON parse error" );
   }
   
   // Set our current frame iterator
   _currentFrameIt = _json.begin();
   if ( _currentFrameIt == _json.end() )
      throw std::runtime_error( "JSON file is empty" );
}
std::unique_ptr< Pose > KpJsonImporter::ReadOne()
{
   // HEADS UP!
   // nlohmann::json conforms to the JSON specification that order doesn't matter;
   // thus, we will iterate frames in the following order (for example):
   //   1
   //  10
   // 100
   // ...etc.
   // There are ways around this but none of them are obvious to me, and
   // I *think* it shouldn't matter since we don't accept JSON files as
   // input streams.
   std::unique_ptr< Pose > returnValue;
   
   if ( _json.empty() )
      throw std::runtime_error( "JSON file not opened or is empty" );

   auto & jsonFrame = *_currentFrameIt++;
   if ( _currentFrameIt == _json.end() )
   {
      _parseComplete = true;
   }
   
   // If we have a valid frame
   if ( !jsonFrame.empty() && jsonFrame.count( "players" ) )
   {
      int timestamp = jsonFrame[ "frameID" ].get< int >();
      
      // For each character in this frame
      for ( auto & jsonCharacter : jsonFrame[ "players" ] )
      {
         std::string name;
         std::string keypointType;

         // Get the basics
         try
         {
            name = jsonCharacter[ "id" ].get<std::string>();
            keypointType = "mop_19";// TODO: This should be specified in the JSON file. Maybe per character or only once; either way I shouldn't need to guess this.
         }
         catch ( std::domain_error & )
         {
            std::stringstream ss;
            ss << "Frame is incomplete";
            throw std::runtime_error( ss.str() );
         }
         
         // Build a keypoint layout and give it to the importer
         std::map< KEYPOINT_TYPE, int > kpLayout = KpImporterFactory::GetKeypointMap( keypointType );
         
         // Make sure we have a valid skeleton entry
         if ( jsonCharacter.count( "skeleton" ) &&
            jsonCharacter[ "skeleton" ].size() >= kpLayout.size() )
         {
            // Create a pose using this information
            returnValue = PoseFactory::Create( keypointType, kpLayout );
            returnValue->Name( name );
            returnValue->Timestamp( timestamp );
            
            // Get the keypoints
            try
            {
               size_t valueIndex = 0;
               for ( auto & jsonKeypoint : jsonCharacter[ "skeleton" ] )
               {
                  std::array< double, 3 > keypointValue = { jsonKeypoint[0].get<double>() * _unitMeterNorm,
                     jsonKeypoint[1].get<double>() * _unitMeterNorm,
                     jsonKeypoint[2].get<double>() * _unitMeterNorm };
                  returnValue->Keypoint( keypointValue, (int)valueIndex++ );
                  
                  // Stop if there are more keypoints than expected
                  if ( valueIndex > kpLayout.size() )
                     break;
               }
            }
            catch ( std::domain_error & )
            {
               std::stringstream ss;
               ss << "Frame is incomplete";
               throw std::runtime_error( ss.str() );
            }
         }
      }
   }
   
   return returnValue;
}
void KpJsonImporter::Close()
{
   if ( !_json.empty() )
      _json.clear();
}
