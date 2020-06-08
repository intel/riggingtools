#include <cstring>
#include <sstream>
#include <thread>
#include "KpImporterFactory.hpp"
#include "PoseFactory.hpp"
#include "KpCsvStreamImporter.hpp"

void KpCsvStreamImporter::Open( std::string filename )
{
   (void)filename;
}
std::unique_ptr< Pose > KpCsvStreamImporter::ReadOne()
{
   bool endOfFile = false; // Always false for this stream
   
   if ( IsParseComplete() )
      throw std::runtime_error( "Cannot read: Parse already complete" );
   
   // If we haven't any data from a previous read
   if ( !_readBuffer.size() )
   {
      const int BUFFER_SIZE = 1 << 19; // ~512KB
      _readBuffer.resize( BUFFER_SIZE );
      
      std::cin.read( (char *)&_readBuffer[0], BUFFER_SIZE );

      // If nothing was read from stdin
      if ( std::cin.gcount() == 0 )
      {
         // Clear everything, respect the CPU
         _readBuffer.clear();
         std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
      }
      else
      {
         // Resize to the exact size read in
         _readBuffer.resize( std::cin.gcount() );
      }
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
void KpCsvStreamImporter::Close()
{
}
