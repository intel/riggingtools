#ifndef KpCsvStreamImporter_hpp
#define KpCsvStreamImporter_hpp

#include "KpCsvImporter.hpp"

class KpCsvStreamImporter : public KpCsvImporter
{
public:
//   KpCsvImporter(){}
//   KpCsvImporter( const KpCsvImporter & rhs );
   virtual ~KpCsvStreamImporter(){}
   
   virtual void Open( std::string filename );
   virtual std::unique_ptr< Pose > ReadOne();
   virtual void Close();
   
   virtual IMPORT_TYPE ParserType() const { return IMPORT_TYPE_CSV_STREAM; }
   
   virtual KpImporter * Clone() const { return new KpCsvStreamImporter( *this ); }
};
#endif
