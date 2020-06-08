#ifndef KpCsvImporter_hpp
#define KpCsvImporter_hpp

#include <stdio.h>
#include <deque>
#include <fstream>
#include <iostream>
#include <array>
#include "Pose.hpp"
#include "KpImporter.hpp"

class KpCsvImporter : public KpImporter
{
public:
   KpCsvImporter(){}
   KpCsvImporter( const KpCsvImporter & rhs );
   virtual ~KpCsvImporter(){}
   
   virtual void Open( std::string filename );
   virtual std::unique_ptr< Pose > ReadOne();
   virtual void Close();
   
   virtual bool IsParseComplete() const { return (_ifstream.eof() && _readBuffer.size() == 0); }
   virtual void KeypointLayout( std::map< KEYPOINT_TYPE, int > value ) { _kpLayout = value; }
   virtual const std::map< KEYPOINT_TYPE, int > & KeypointLayout() const { return _kpLayout; }
   virtual IMPORT_TYPE ParserType() const { return IMPORT_TYPE_CSV; }
   virtual double UnitMeterNorm() const { return _unitMeterNorm; }
   virtual void UnitMeterNorm(double v) { _unitMeterNorm = v; }
   
   virtual KpImporter * Clone() const { return new KpCsvImporter( *this ); }

protected:
   virtual bool ParseKeypointType( const uint8_t * data,
      size_t dataSize,
      std::string & type );
   size_t ParseHeader( Pose * poseClass,
      const uint8_t * data,
      size_t dataSize );
   size_t ParsePoseData( Pose * poseClass,
      const uint8_t * data,
      size_t dataSize,
      bool endOfInput );
   bool IsHeaderParsed() const { return _parseState != HEADER; }
   
   std::unique_ptr< Pose > _currentPose;
   std::vector< uint8_t > _readBuffer;
   size_t _numParsedDoubles = 0;
   
private:
   enum PARSE_STATE
   {
      HEADER,
      COMMA,
      DATA,
      NEWLINE,
      UNKNOWN
   } _parseState = HEADER;
   
   std::vector< uint8_t > _bufferedParseData;
   std::map< KEYPOINT_TYPE, int > _kpLayout;
   std::array< double, 3 > _currentValue;
   std::ifstream _ifstream;
   double _unitMeterNorm = 1.0;
};
#endif
