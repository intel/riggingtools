#include <sstream>
#include <fstream>
#include <json.hpp>
#include "Utility.hpp"
#include "KpImporterFactory.hpp"

// Built-in keypoint importers
#include "KpCsvImporter.hpp"
#include "KpCsvStreamImporter.hpp"
#include "KpJsonImporter.hpp"

// Support old versions of nlohmann JSON parser
typedef typename std::conditional< (NLOHMANN_JSON_VERSION_MAJOR < 3), typename std::invalid_argument, typename nlohmann::json::parse_error >::type jsonParseException_t;

// Static JSON object loaded from kpDescriptor.json
nlohmann::json g_kpDescriptor;

KpImporter::IMPORT_TYPE KpImporterFactory::DetermineType( std::string filename )
{
   std::string filenameExtension = Utility::GetFilenameExtension( filename );
   
   if ( filename == "" ||
      filename == "stdin" )
      return KpImporter::IMPORT_TYPE_CSV_STREAM;
   else if ( filenameExtension == "json" )
      return KpImporter::IMPORT_TYPE_JSON;
   
   // Assume CSV otherwise
   return KpImporter::IMPORT_TYPE_CSV;
}
std::unique_ptr< KpImporter > KpImporterFactory::Create( KpImporter::IMPORT_TYPE type )
{
   // Create a new importer
   switch ( type )
   {
      case KpImporter::IMPORT_TYPE_CSV: return std::unique_ptr< KpImporter >( new KpCsvImporter() );
      case KpImporter::IMPORT_TYPE_CSV_STREAM: return std::unique_ptr< KpImporter >( new KpCsvStreamImporter() );
      case KpImporter::IMPORT_TYPE_JSON: return std::unique_ptr< KpImporter >( new KpJsonImporter() );
      default: return std::unique_ptr< KpImporter >( nullptr );
   }
}
std::map< KEYPOINT_TYPE, int > KpImporterFactory::GetKeypointMap( std::string kpType )
{
   std::map< KEYPOINT_TYPE, int > returnValue;
   std::vector< std::string > jsonFilenames = { "kpDescriptor.json" };
   
   // If our JSON file hasn't been loaded yet
   if ( g_kpDescriptor == nullptr )
   {
      // Include the binary path when searching for the JSON file
      std::string applicationDirectory;
      if ( Utility::GetApplicationDirectory( applicationDirectory ) )
      {
         jsonFilenames.push_back( applicationDirectory + "/kpDescriptor.json" );
      }
      
      for ( auto & filename : jsonFilenames )
      {
         // Try to open the file. If it opened
         std::ifstream i( filename.c_str() );
         if ( i.good() )
         {
            // Try to parse the JSON file
            try
            {
               i >> g_kpDescriptor;
            }
            catch ( jsonParseException_t e )
            {
               g_kpDescriptor = nullptr;
               std::stringstream ss;
               ss << "Could not parse '" << filename << "': " << e.what();
               throw std::runtime_error( ss.str() );
            }
            break;
         }
      }
   }
   
   // If we couldn't find a suitable file
   if ( g_kpDescriptor == nullptr )
   {
      std::stringstream ss;
      ss << "Could not find '" << jsonFilenames[0] << "'";
      throw std::runtime_error( ss.str() );
   }
   // Find the keypoint descriptor for this type
   auto it = g_kpDescriptor.find( kpType );
   if ( it == g_kpDescriptor.end() )
   {
      // Print a truncated section of the type in case it's really long and not a type at all
      std::stringstream ss;
      std::string firstFewCharacters = kpType.substr(0, 20);
      if ( firstFewCharacters.length() != kpType.length() )
         firstFewCharacters +="...";
      ss << "Bad 'type' field in input file: kpDescriptor.json does not contain a section for '" << firstFewCharacters << "'";
      throw std::runtime_error( ss.str() );
   }
   
   // Iterate through the keypoint layout IN FILE ORDER.
   int index = 0;
   for ( auto kp : (*it)[ "layout" ] )
   {
      returnValue[ StrToKpType( kp ) ] = index++;
   }
   
   return returnValue;
}

