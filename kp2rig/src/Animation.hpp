#ifndef Animation_hpp
#define Animation_hpp

#include <stdio.h>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <json.hpp>
#include "AnimatedRig.hpp"

class Animation
{
public:
   Animation( double targetFps );
   ~Animation();
   
   double SegmentDuration() const { return _segmentDuration; }
   void SegmentDuration( double v ) { _segmentDuration = v; }
   void OutputDirectory( std::string v ) { _outputDirectory = v; }
   void Smooth( SMOOTH_TYPE v ) { _smoothType = v; }
   void MaxMissingFrameGap( double v ) { _maxMissingFrameGap = v; }
   const std::vector< std::string > & SegmentFilenames() const;
   void FlushSegments();
   
   void AddPose( std::string rigId,
      std::unique_ptr< Pose > & pose );
   const std::map< std::string, AnimatedRig > & GetAnimatedRigs() const;
   
private:
   std::vector< std::pair< int, int > > GetBounds( int & start,
      int & end );
   void WriteForever();
   void ProcessAndWrite( int startTimestamp,
      int endTimestamp,
      bool flush = false );
   
   double _segmentDuration;
   std::map< std::string, AnimatedRig > _animatedRigs;
   std::vector< std::string > _segmentFilenames;
   std::thread _thread;
   std::mutex _mutex;
   std::condition_variable _flushEvent;
   volatile bool _quit = false;
   double _fps;
   std::unique_ptr< nlohmann::json > _json;
   std::string _outputDirectory;
   double _maxMissingFrameGap = 0.5;
   volatile bool _flush = false;
   SMOOTH_TYPE _smoothType = SMOOTH_TYPE_NONE;
};
#endif /* Animation_hpp */

inline const std::map< std::string, AnimatedRig > & Animation::GetAnimatedRigs() const
{
   return _animatedRigs;
}
inline const std::vector< std::string > & Animation::SegmentFilenames() const
{
   return _segmentFilenames;
}
inline void Animation::FlushSegments()
{
   // Mark for writing
   _flush = true;
   
   // Wait for the flush to complete for safety
   std::unique_lock< std::mutex > waitLock( _mutex );
   _flushEvent.wait( waitLock );
}
