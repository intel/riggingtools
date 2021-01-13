#ifndef KpJsonImporter_hpp
#define KpJsonImporter_hpp

#include <stdio.h>
#include <vector>
#include <json.hpp>
#include <memory>
#include "Pose.hpp"
#include "KpImporter.hpp"

class KpJsonImporter : public KpImporter
{
public:
   virtual ~KpJsonImporter(){}
   
   virtual void Open( std::string filename );
   virtual std::unique_ptr< Pose > ReadOne();
   virtual void Close();
   
   virtual bool IsParseComplete() const { return _parseComplete; }
   virtual void KeypointLayout( std::map< KEYPOINT_TYPE, int > value ) { _kpLayout = value; }
   virtual const std::map< KEYPOINT_TYPE, int > & KeypointLayout() const { return _kpLayout; }
   virtual IMPORT_TYPE ParserType() const { return IMPORT_TYPE_JSON; }
   virtual double UnitMeterNorm() const { return _unitMeterNorm; }
   virtual void UnitMeterNorm(double v) { _unitMeterNorm = v; }
   
   virtual KpImporter * Clone() const { return new KpJsonImporter( *this ); }

private:
   void OpenJson( std::string jsonFilename );
   void UpdateCurrentPlayerIt ();
   
   nlohmann::json _json;
   nlohmann::json::iterator _currentFrameIt;
   nlohmann::json::iterator _currentPlayerIt;
   bool _parseComplete = false;
   std::map< KEYPOINT_TYPE, int > _kpLayout;
   double _unitMeterNorm = 1.0;
};
#endif
