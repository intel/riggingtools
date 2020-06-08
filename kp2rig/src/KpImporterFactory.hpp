#ifndef KpImporterFactory_hpp
#define KpImporterFactory_hpp

#include <string>
#include <memory>
#include <map>
#include "KpImporter.hpp"

class KpImporterFactory
{
public:
   static KpImporter::IMPORT_TYPE DetermineType( std::string filename );
   static std::unique_ptr< KpImporter > Create( KpImporter::IMPORT_TYPE type );
   static std::map< KEYPOINT_TYPE, int > GetKeypointMap( std::string kpType );
};

#endif
