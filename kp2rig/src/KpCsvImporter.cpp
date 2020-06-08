#include <cstring>
#include <sstream>
#include "KpImporterFactory.hpp"
#include "PoseFactory.hpp"
#include "KpCsvImporter.hpp"

KpCsvImporter::KpCsvImporter( const KpCsvImporter & rhs )
{
   // Only copy the kp layout
   _kpLayout = rhs._kpLayout;
}
void KpCsvImporter::Open( std::string filename )
{
   _ifstream.open( filename );
   if ( !_ifstream.good() )
   {
      std::stringstream ss;
      ss << "Could not open file '" << filename << "'";
      throw std::runtime_error( ss.str() );
   }
}
std::unique_ptr< Pose > KpCsvImporter::ReadOne()
{
   bool endOfFile = false;
   
   if ( IsParseComplete() )
      throw std::runtime_error( "Cannot read: Parse already complete" );
   
   // If we haven't any data from a previous read
   if ( !_readBuffer.size() )
   {
      if ( !_ifstream.good() )
         throw std::runtime_error( "Cannot read: File not opened or EOF" );
      
      const int BUFFER_SIZE = 1 << 19; // ~512KB
      _readBuffer.resize( BUFFER_SIZE );
   
      // Read a chunk
      if ( !_ifstream.good() ||
         (!_ifstream.read( (char *)&_readBuffer[0], BUFFER_SIZE ) && !_ifstream.gcount()) )
      {
         std::stringstream ss;
         ss << "Cannot read file '" << ss.str() << "'";
         throw std::runtime_error( ss.str() );
      }
      
      // Resize to the exact size read in
      _readBuffer.resize( _ifstream.gcount() );
      
      // Mark whether or not this is the end of file
      endOfFile = _ifstream.eof();
   }
   
   size_t bytesRead = (size_t)_readBuffer.size();
   uint8_t * data = &_readBuffer[0];

   // If we have data
   size_t offset = 0;
   std::string keypointType = "";
   if ( bytesRead )
   {
      // If we don't have a current pose
      if ( _currentPose == nullptr )
      {
         // Parse for the keypoint field. If we find it
         if ( ParseKeypointType( data,
            bytesRead,
            keypointType ) )
         {
            // Build a keypoint layout and give it to the importer
            std::map< KEYPOINT_TYPE, int > kpLayout = KpImporterFactory::GetKeypointMap( keypointType );

            // Create a pose using this information
            _currentPose = PoseFactory::Create( keypointType, kpLayout );
         }
         // Else we need more data
         else
         {
            _readBuffer.clear();
         }
      }
      
      // If we have a current pose
      if ( _currentPose != nullptr )
      {
         if ( !IsHeaderParsed() )
            offset += ParseHeader( _currentPose.get(),
               data,
               bytesRead );
         
         if ( offset < bytesRead )
            offset += ParsePoseData( _currentPose.get(),
               data + offset,
               bytesRead - offset,
               endOfFile );
      }
      
      // Remove the parsed data, if any
      if ( offset > 0 )
      {
         _readBuffer.erase( std::begin(_readBuffer),
            std::begin(_readBuffer) + offset );
      }
   }
   
   if ( _currentPose != nullptr &&
      _numParsedDoubles == _currentPose->InputDataSize() )
      return std::move( _currentPose );
   else
      return nullptr;
}
void KpCsvImporter::Close()
{
   if ( _ifstream.good() )
      _ifstream.close();
}

bool KpCsvImporter::ParseKeypointType( const uint8_t * data,
   size_t dataSize,
   std::string & type )
{
   // Assuming the data is formatted correctly, data should point to either the first character in the line or
   // a fragment of data continuing the previous.
   size_t position1 = 0;
   size_t position2 = 0;
   _numParsedDoubles = 0;
   
   // If we find a comma
   const uint8_t * comma = (const uint8_t *)memchr( data, ',', dataSize );
   if ( comma )
   {
      std::string frameHeaderData = "";
      
      // If we have previous data
      if ( _bufferedParseData.size() )
      {
         // Prepend that data to our string
         frameHeaderData = std::string( (const char *)&_bufferedParseData[0], _bufferedParseData.size() );
         _bufferedParseData.clear();
      }
      
      // Add this header data to our string
      frameHeaderData += std::string( (const char *)data, comma - data + 1 );
      
      // Token: keypoint type. Burn whitespace, don't include the comma
      while ( isspace(frameHeaderData[position1]) ) ++position1;
      position2 = frameHeaderData.find_first_of( ',', position1 );
      type = frameHeaderData.substr( position1, position2 - position1 );
      
      return true;
   }
   else
   {
      // Store the partial value
      _bufferedParseData.resize( dataSize );
      memcpy( &_bufferedParseData[0],
         data,
         dataSize );
      
      return false;
   }
}
size_t KpCsvImporter::ParseHeader( Pose * poseClass,
   const uint8_t * data,
   size_t dataSize )
{
   size_t position1 = 0;
   size_t position2 = 0;
   _numParsedDoubles = 0;
   
   // If we find a colon
   const uint8_t * colon = (const uint8_t *)memchr( data, ':', dataSize );
   if ( colon )
   {
      std::string frameHeaderData = "";
      
      // If we have previous data
      if ( _bufferedParseData.size() )
      {
         // Prepend that data to our string
         frameHeaderData = std::string( (const char *)&_bufferedParseData[0], _bufferedParseData.size() );
         _bufferedParseData.clear();
      }
      
      // Add this header data to our string
      frameHeaderData += std::string( (const char *)data, colon - data + 1 );
      
      // Token: kp type, which is already known so we don't need it.
      // Don't include the comma
      position2 = frameHeaderData.find_first_of( ',', position1 );
      position1 = position2 + 1;
      
      // Token: character name. Burn whitespace, don't include the comma
      while ( isblank(frameHeaderData[position1]) ) ++position1;
      position2 = frameHeaderData.find_first_of( ',', position1 );
      poseClass->Name( frameHeaderData.substr( position1, position2 - position1 ) );
      position1 = position2 + 1;
      
      // Token: custom data. TODO: Currently ignored, we should probably make a member to store this!
      position2 = frameHeaderData.find_first_of( ',', position1 );
      position1 = position2 + 1;
      
      // Token: frame number, burn whitespace,
      while ( isblank(frameHeaderData[position1] ) ) ++position1;
      position2 = frameHeaderData.find_first_of( ':', position1 );
      poseClass->Timestamp( std::stoi( frameHeaderData.substr( position1, position2 - position1 ) ) );
      _parseState = COMMA;
      
      // +1 to skip over the colon delimiter
      return (colon - data) + 1;
   }
   else
   {
      // Store the partial value
      _bufferedParseData.resize( dataSize );
      memcpy( &_bufferedParseData[0],
         data,
         dataSize );
      
      return dataSize;
   }
}
size_t KpCsvImporter::ParsePoseData( Pose * poseClass,
   const uint8_t * data,
   size_t dataSize,
   bool endOfInput )
{
   size_t offset = 0;
   while ( _numParsedDoubles < _currentPose->InputDataSize() )
   {
      // Stop if we are out of data
      if ( offset >= dataSize )
         break;
         
      switch ( _parseState )
      {
         case NEWLINE:
         case COMMA:
         {
            // Burn whitespace, commas, and newlines
            while ( offset < dataSize &&
               (isblank(data[offset]) || isspace(data[offset]) || data[offset] == ',' ) ) ++offset;
            
            // Only move to the DATA state if we see data
            if ( offset < dataSize )
               _parseState = DATA;
            break;
         }
         case DATA:
         {
            // Find the end of the data
            const uint8_t * endOfData = data + offset;
            while ( size_t(endOfData - data) < dataSize &&
               (isalnum(*endOfData) || *endOfData == '.' || *endOfData == '-') ) ++endOfData;
               
            // If we didn't find the end of the data
            if ( !endOfInput && size_t(endOfData - data) >= dataSize )
            {
               // Store the partial value
               _bufferedParseData.resize( dataSize - offset );
               memcpy( &_bufferedParseData[0],
                  data + offset,
                  dataSize - offset );
               
               offset = dataSize;
            }
            // Else we did find the end of the data
            else
            {
               std::string stringValue = "";
               
               // If we have buffered parse data from earlier
               if ( _bufferedParseData.size() > 0 )
               {
                  stringValue += std::string( (const char *)&_bufferedParseData[0], _bufferedParseData.size() );
                  _bufferedParseData.clear();
               }
               
               stringValue += std::string( (const char *)data + offset, (endOfData - (data + offset)) );
                  
               double doubleValue = 0.0;
               try
               {
                  doubleValue = std::stod( stringValue ) * _unitMeterNorm;
               }
               catch ( std::invalid_argument e )
               {
                  std::stringstream ss;
                  ss << "Could not parse `" << stringValue.substr( 0, 20 ) << "` as a valid floating-point value. Is this data really `" << poseClass->KpType() << "'?";
                  throw std::runtime_error( ss.str() );
               }
               
               // We need 3 doubles to make a single XYZ value, update the single component here.
               // Also post-increment our running total
               int valueIndex = (int)_numParsedDoubles / 3;
               int componentIndex = (int)_numParsedDoubles % 3;
               _currentValue[ componentIndex ] = doubleValue;
               
               // Update the pose when we have all doubles for this value
               if ( componentIndex == 2 )
                  poseClass->Keypoint( _currentValue, valueIndex );
               
               // Move to the end
               offset = size_t(endOfData - data) + 1;
               
               ++_numParsedDoubles;
               _parseState = COMMA;
            }
            break;
         }
         default: break;
      }
   }
   
   if ( _numParsedDoubles == _currentPose->InputDataSize() )
   {
      _parseState = HEADER;
   }
   
   return offset;
}
