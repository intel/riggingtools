#ifndef KpImporter_h
#define KpImporter_h

#include <map>
#include "KpType.hpp"

class Pose;

// This is the interface you will implement if you make a custom importer
class KpImporter
{
public:
   enum IMPORT_TYPE
   {
      IMPORT_TYPE_JSON,
      IMPORT_TYPE_CSV,
      IMPORT_TYPE_CSV_STREAM,
      IMPORT_TYPE_UNKNOWN
   };

   virtual ~KpImporter(){}
   
   virtual void Open( std::string filename ) = 0;
   virtual std::unique_ptr< Pose > ReadOne() = 0;
   virtual void Close() = 0;
   virtual bool IsParseComplete() const = 0;
   virtual void KeypointLayout( std::map< KEYPOINT_TYPE, int > value ) = 0;
   virtual const std::map< KEYPOINT_TYPE, int > & KeypointLayout() const = 0;
   virtual IMPORT_TYPE ParserType() const = 0;
   virtual double UnitMeterNorm() const = 0;
   virtual void UnitMeterNorm(double v) = 0;
   
   // Prototype pattern
   virtual KpImporter * Clone() const = 0;
};

#endif
