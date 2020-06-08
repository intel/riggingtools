#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits.h>
#include <functional>
#include "Animation.hpp"
#include "RigPose.hpp"
#include "Utility.hpp"
#include "config.h"

Animation::Animation( double targetFps )
   : _fps( targetFps )
{
   // We do everything on a worker thread
   _thread = std::thread( [this]{ this->WriteForever(); } );
}
Animation::~Animation()
{
   // Finish writing everything we have
   FlushSegments();
   
   // Exit our worker thread
   _quit = true;
   _thread.join();
}
void Animation::AddPose( std::string rigId,
   std::unique_ptr< Pose > & pose )
{
   std::lock_guard< std::mutex > lock( _mutex );
   
   auto it = _animatedRigs.find( rigId );
   if ( it == std::end( _animatedRigs ) )
      it = _animatedRigs.emplace( std::make_pair( rigId, AnimatedRig() ) ).first;
   
   (*it).second.AddPose( pose );
}

std::vector< std::pair< int, int >  > Animation::GetBounds( int & start,
   int & end )
{
   std::vector< std::pair< int, int >  > returnValue;
   start = INT_MAX;
   end = 0;
   
   std::lock_guard< std::mutex > autoLock( _mutex );
   
   for ( auto & animatedRig : _animatedRigs )
   {
      if ( animatedRig.second.GetFrames().size() )
      {
         int thisCharacterStart = (*animatedRig.second.GetFrames().begin()).second->Timestamp();
         int thisCharacterEnd = (*animatedRig.second.GetFrames().rbegin()).second->Timestamp();
         returnValue.push_back( std::make_pair( thisCharacterStart, thisCharacterEnd ) );
         
         // Keep track of overall min/max
         if ( thisCharacterStart < start ) start = thisCharacterStart;
         if ( thisCharacterEnd > end ) end = thisCharacterEnd;
      }
   }
   
   return returnValue;
}
void Animation::WriteForever()
{
   int min, max;
   int segmentStartTimestamp = -1;
   int segmentEndTimestamp;

   // Loop until told to stop OR until told to stop and all data is written
   while ( !_quit )
   {
      // Get the bounds of all animated rigs
      std::vector< std::pair< int, int >  > bounds = GetBounds( min, max );
      
      // If we have bounds
      if ( bounds.size() )
      {
         std::lock_guard< std::mutex > autoLock( _mutex );
         
         // Calculate the min, max, and min/max averages
         int minSum = 0, maxSum = 0;
         for ( auto & bound : bounds )
         {
            if ( bound.first < min ) min = bound.first;
            if ( bound.second > max ) max = bound.second;
            
            minSum += bound.first;
            maxSum += bound.second;
         }
         double minAverage = minSum / (int)bounds.size();
         double maxAverage = maxSum / (int)bounds.size();
         
         // Set our end timestamp to the end, we can move it later if needed
         segmentEndTimestamp = max;
      
         // If we are streaming (using segments)
         if ( _segmentDuration > 0 )
         {
            // For streaming, force our segment timestamp ONLY the first time here;
            // subsequent iterations MUST resume where the previous segment left off
            if ( segmentStartTimestamp < 0 )
            {
               segmentStartTimestamp = min;
            }
            
            // If we have enough data to output a segment
            int targetDiff = (int)(_fps * _segmentDuration) - 1;
            if ( (max - min) >= targetDiff &&
               ((maxAverage - minAverage) >= targetDiff || (max - min) >= (targetDiff * 3) / 2) )
            {
               // Adjust our end timestamp and mark for writing
               segmentEndTimestamp = segmentStartTimestamp + targetDiff;
               
               _flush = true;
            }
         }
         // Else we are outputting to a monolithic file
         else
         {
            // Force our segment timestamp every iteration.
            // This addresses a case where the last file has the earliest timestamp,
            // a case we don't (and can't) worry about while stremaing.
            segmentStartTimestamp = min;
         }
         
         // If we need to write
         if ( _flush )
         {
            printf( "Processing and writing segment %d-->%d...", segmentStartTimestamp, segmentEndTimestamp );
           
            // Write this data, move the start timestamp to one passed the end
            try
            {
               ProcessAndWrite( segmentStartTimestamp, segmentEndTimestamp, _flush );
               printf( "DONE\n" );
            }
            catch ( std::runtime_error & e )
            {
               printf( "FAILED\n\t%s\n", e.what() );
            }
            segmentStartTimestamp = segmentEndTimestamp + 1;
         }
      }

      // If we were asked to flush
      if ( _flush )
      {
         // When we get here the flush has been handled or didn't apply; either way it's been addressed.
         // Mark the flush as complete.
         _flush = false;
         _flushEvent.notify_one();
      }
      
      // This thread will have mutex issues and poor performance if left
      // to run as fast as possible, so release resources to speed up the overall architecture
      std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
   }
}
void Animation::ProcessAndWrite( int startTimestamp,
   int endTimestamp,
   bool flush )
{
   RAII scopeGuard( [ flush, this ]
   {
      // If we flushed a file (whether or not it actually succeeded)
      if ( flush )
      {
         // Destroy this JSON file
         this->_json.reset();
      }
   } );
   
   // If we have a json object and we are writing segments
   if ( _json && _segmentDuration > 0 )
   {
      // If we need to close this file and open a new one
      if ( startTimestamp > (*_json)["header"]["endFrame"].get<int>() )
      {
         // Create the segment filename and add to our list
         std::stringstream ss;
         ss << _outputDirectory << "/" << "seg_" << (*_json)["header"]["startFrame"] << ".json";
         _segmentFilenames.push_back( ss.str() );
         
         // Write this file to disk
         std::ofstream fileStream( ss.str() );
         fileStream << (*_json);
         
         // Destroy this json file
         _json.reset();
      }
   }
   
   // If we don't have a json object
   if ( !_json )
   {
      // Create one
      _json = std::unique_ptr< nlohmann::json >( new nlohmann::json() );
      
      // Write the header
      //#define XSTR(s) STR(s)
      //#define STR(s) #s
      //(*_json)["version"] = XSTR(MY_VERSION);
      (*_json)["version"] = MY_VERSION;
      (*_json)["header"]["startFrame"] = startTimestamp;
      (*_json)["header"]["endFrame"] = endTimestamp;
      (*_json)["header"]["fps"] = _fps;
      
      // Start the frames array
      (*_json)["rigs"] = {};
   }
   
   // For every rig
   for ( auto & animatedRig : _animatedRigs )
   {
      // Make sure we have some frames first
      if ( animatedRig.second.GetFrames().size() == 0 )
         continue;
         
      // Find this character in the json file
      nlohmann::json * existingRig = nullptr;
      for ( auto & rig : (*_json)["rigs"] )
      {
         if ( rig["id"] == animatedRig.first )
         {
            existingRig = &rig;
            break;
         }
      }
      
      // If not found
      if ( existingRig == nullptr )
      {
         // Create a new character and add it to our json file
         nlohmann::json rig;
         rig["id"] = animatedRig.first;
         rig["type"] = animatedRig.second.Category();
         rig["name"] = animatedRig.first;
         (*_json)["rigs"].push_back( rig );
         existingRig = &(*_json)["rigs"].back();
      }

      // "Fix" any missing frames
      animatedRig.second.FixMissingFrames( startTimestamp,
         endTimestamp,
         (int)((_maxMissingFrameGap * _fps) + 0.5) );
         
      int firstTimestamp;
      int lastTimestamp;
      int actualStart = startTimestamp, actualEnd = endTimestamp;
      
      // Smooth noisy motion, if requested by the _smoothType
      animatedRig.second.SmoothFrames( _smoothType,
         actualStart,
         actualEnd,
         flush );
      
      // Write this character's data
      firstTimestamp = (*animatedRig.second.GetFrames().begin()).second->Timestamp();
      lastTimestamp = animatedRig.second.Write( *existingRig,
         startTimestamp,
         endTimestamp );
      
      // Update the bounds for this character if they don't match the global bounds.
      // Note this should happen AFTER frame interpolation and be able to account
      // for frames from previous iterations
      if ( firstTimestamp > startTimestamp )
         (*existingRig)["startFrame"] = firstTimestamp;
      if ( lastTimestamp < endTimestamp )
         (*existingRig)["endFrame"] = lastTimestamp;
   }
   
   // If we need to write this json object
   if ( flush )
   {
      // Create the segment filename and add to our list now
      std::stringstream ss;
      ss << _outputDirectory << "/" << "seg_" << (*_json)["header"]["startFrame"] << ".json";
      
      std::string outputFilename = Utility::ExpandTilde( ss.str() );
      
      // Write this file to disk
      std::ofstream fileStream( outputFilename );
      if ( fileStream.good() )
      {
         fileStream << (*_json);
         _segmentFilenames.push_back( ss.str() );
      }
      else
      {
         char buffer[512];
         buffer[0] = 0;
         auto dontCare = strerror_s( buffer, sizeof( buffer ), errno ); (void)dontCare;
         ss.str( std::string() ); // Clear the string stream
         ss << "Tried to write '" << "seg_" << (*_json)["header"]["startFrame"] << ".json' to '" << _outputDirectory << "': " << buffer;
         throw std::runtime_error( ss.str() );
      }
   }
}
