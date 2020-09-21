#define _USE_MATH_DEFINES
#include <cmath>
#include "AnimatedRig.hpp"
#include "Utility.hpp"
#include "Compression.hpp"
#include "PoseFactory.hpp"
#include "RigPose.hpp"

// This defines the "window" size of our low-pass filter
const int NUM_TAPS = 21;

AnimatedRig::AnimatedRig()
   : _rawBoneLengths( Rig::MAX_NUM_JOINTS + Rig::MAX_NUM_JOINT_OFFSETS, std::vector< double >() )
{
}

void AnimatedRig::AddPose( std::unique_ptr< Pose > & pose )
{
   // Set our type if not set
   if ( _category.empty() )
      _category = pose->Category();
      
   auto it = _frames.emplace( std::make_pair( pose->Timestamp(), std::move( pose ) ) ).first;

   DetermineBoneLengths( it );
}
void AnimatedRig::DetermineBoneLengths( std::map< int, std::unique_ptr< Pose > >::iterator & poseIt )
{
   // Store the first MIN_NUM_BONES bone lengths
   const int MIN_NUM_BONES = 5;
   if ( _rawBoneLengths[ 0 ].size() < MIN_NUM_BONES )
   {
      // Generate a RigPose only so we can determine bone lengths
      (*poseIt).second->GenerateRig();
      
      // Ensure the rig is good-to-go
      if ( !(*poseIt).second->ValidateRig() )
      {
         std::stringstream ss;
         ss << (*poseIt).second->Name() << ": Rig is invalid! This can be caused by bad math, or missing keys in kpDescriptor.json";
         throw std::runtime_error( ss.str().c_str() );
      }
   
      int boneIndex = 0;
      const Rig & rig = (*poseIt).second->RigPose().GetRig();
      
      // Bone lengths
      for ( int i = 0; i < rig.numJointsUsed; ++i )
      {
         const Joint & joint = rig.GetJoint( Rig::JOINT_TYPE(i) );
         _rawBoneLengths[ boneIndex++ ].push_back( joint.length );
      }
      
      // Offset lengths, calculated from vectors
      for ( int i = 0; i < rig.numJointOffsetsUsed; ++i )
      {
         const JointOffset & jointOffset = rig.GetJointOffset( Rig::JOINT_OFFSET_TYPE(i) );
         _rawBoneLengths[ boneIndex++ ].push_back( Utility::RawToVector( jointOffset ).norm() );
      }
   }
   
   // If we haven't calculated our final bone lengths
   auto it = poseIt;
   if ( _averagedBoneLengths.size() == 0 )
   {
      // If we have enough data to calculate our average bone lengths
      if (  _rawBoneLengths[ 0 ].size() == MIN_NUM_BONES )
      {
         // For every bone
         for ( auto rawBoneLength : _rawBoneLengths )
         {
            // Compute and set the average
            auto sum = 0.0;
            for ( auto val : rawBoneLength )
               sum += val;
            _averagedBoneLengths.push_back( sum / rawBoneLength.size() );
         }
         
         // Update all existing poses
         it = _frames.begin();
      }
   }
}
void AnimatedRig::UpdateBoneLengths( std::unique_ptr< Pose > & pose )
{
   // If we have our average bone lengths
   if ( _averagedBoneLengths.size() > 0 )
   {
      int boneIndex = 0;
      Rig & rig = pose->RigPose().GetRig();

      // Bone lengths
      for ( int i = 0; i < rig.numJointsUsed; ++i )
      {
         Joint & joint = rig.GetJoint( Rig::JOINT_TYPE(i) );
         joint.length = _averagedBoneLengths[ boneIndex++ ];
      }

      // Adjusted offsets (calculated from lengths)
      for ( int i = 0; i < rig.numJointOffsetsUsed; ++i )
      {
         JointOffset jointOffset = rig.GetJointOffset( Rig::JOINT_OFFSET_TYPE(i) );
         Eigen::Vector3d vec = Utility::RawToVector( jointOffset ).normalized();
         jointOffset = Utility::VectorToRaw( vec * _averagedBoneLengths[ boneIndex++ ] );
      }
   }
}
void AnimatedRig::FixMissingFrames( int rangeStart,
   int rangeEnd,
   int missingFramesThreshold )
{
   int previousTimestamp = rangeStart - 1;
   
   auto it = _frames.begin();
   while ( it != _frames.end() )
   {
      auto timestamp = (*it).first;
      if ( timestamp >= rangeStart &&
         timestamp <= rangeEnd )
      {
         // See how many frames we are missing
         int numMissingFrames = timestamp - previousTimestamp - 1;
         
         // If we missed some frames
         if ( numMissingFrames > 0 )
         {
            // If we are mising a bunch at the beginning
            if ( previousTimestamp == rangeStart - 1 )
            {
               // Don't fix anything, this will be handled by the adjusted bounds               
               ++it;
               previousTimestamp = timestamp;
               continue;
            }
            // Else if we can fix this cleanly
            else if ( numMissingFrames <= missingFramesThreshold )
            {
               // Get the 2 rigs we will interpolate betwen, making sure to generate rigs
               // This will be done on the quaternions, not the points
               RigPose & lhs = _frames[ previousTimestamp ]->GenerateRig();
               const RigPose & rhs = (*it).second->GenerateRig();
               std::string kpType = (*it).second->KpType();

               // Interpolate frames here
               for ( int i = 0; i < numMissingFrames; ++i )
               {
                  // This defines how the lhs and rhs are weighted
                  double ratio = double(i + 1) / (double)(numMissingFrames + 1);

                  // Interpolate, make a unique pointer, update the timestamp, add to our list.
                  // Reset our iterator since we modified the list
                  std::unique_ptr< RigPose > copy( new RigPose( lhs.Interpolate( rhs, ratio ) ) );
                  copy->Timestamp( previousTimestamp + 1 + i );
                  std::unique_ptr< Pose > interpolatedFrame = PoseFactory::FromRigPose( *copy, kpType, (*it).second->KeypointLayout() );
                  
                  _frames.emplace( std::make_pair( copy->Timestamp(), std::move( interpolatedFrame ) ) );
               }

               // Our original iterator is probably invalid, so re-point it here so we can continue iterating
               it = _frames.find( timestamp );
            }
            // Else too many missing contiguous frames - enter damage control mode!
            else
            {
               // Get the 2 rigs we will interpolate betwen, making sure to generate rigs
               // This will be done on the quaternions, not the points
               RigPose & lhs = _frames[ previousTimestamp ]->GenerateRig();
               std::string kpType = (*it).second->KpType();
               
               // Anything we do may make things worse, so just copy the same frame over and over
               for ( int i = 0; i < numMissingFrames; ++i )
               {
                  // Reset our iterator since we modified the list
                  std::unique_ptr< RigPose > copy( new RigPose( lhs ) );
                  copy->Timestamp( previousTimestamp + 1 + i );
                  std::unique_ptr< Pose > interpolatedFrame = PoseFactory::FromRigPose( *copy, kpType, (*it).second->KeypointLayout() );
                  _frames.emplace( std::make_pair( copy->Timestamp(), std::move( interpolatedFrame ) ) );
               }
               
               // Our original iterator is probably invalid, so re-point it here so we can continue iterating
               it = _frames.find( timestamp );
            }
         }
      }
      
      ++it;
      previousTimestamp = timestamp;
   }
}
void AnimatedRig::SmoothFrames( SMOOTH_TYPE type,
   int & rangeStart,
   int & rangeEnd,
   bool flush )
{
   // Don't need to do anything if smoothing is disabled
   if ( type == SMOOTH_TYPE_NONE ||
      type == SMOOTH_TYPE_UNKNOWN )
   {
      return;
   }
   
   // Filter XYZ data first, ensuring new rigs are generated for each filtered frame
   SmoothAllKeypoints( type,
      rangeStart,
      rangeEnd,
      flush );
   
   // Then filter bone roll, because rolls only exist after we have rigs
   SmoothAllBoneRolls( type,
      rangeStart,
      rangeEnd,
      flush );
}
void AnimatedRig::SmoothAllKeypoints( SMOOTH_TYPE type,
   int & rangeStart,
   int & rangeEnd,
   bool flush )
{
   // Derived from data analysis, this is the smooth factor
   const double NORMALIZED_FREQUENCY = 1./10.;

   int actualRangeStart = rangeEnd + 1;
   int actualRangeEnd = rangeStart - 1;
   
   // Sanity check
   if ( !_frames.size() )
      return;

   // -------------------------------------------------
   // ITERATION 1: Add original XYZ samples from every frame
   // -------------------------------------------------
   for ( const auto & frame : _frames )
   {
      auto timestamp = frame.first;
      if ( timestamp >= rangeStart &&
         timestamp <= rangeEnd )
      {
         // Keep track of actual timestamps
         if ( timestamp < actualRangeStart )
            actualRangeStart = timestamp;
         if ( timestamp > actualRangeEnd )
            actualRangeEnd = timestamp;
            
         // Get the input XYZ data, serialized
         std::vector< double > data;
         frame.second->InputDataToArray( data );

         // For every value in the XYZ data
         for ( size_t i = 0; i < data.size(); ++i )
         {
            // Lazy initialization of an LPF object
            if ( i >= _jointSmoothers.size() )
            {
               auto filter = SmoothFactory::Create( type );
               filter->Initialize( NUM_TAPS, NORMALIZED_FREQUENCY );
               _jointSmoothers.emplace_back( std::move(filter) );
            }
            _jointSmoothers[ i ]->AddSample( timestamp, data[ i ] );
         }
      }
   }

   if ( actualRangeEnd - actualRangeStart > 0 )
   {
      // -------------------------------------------------
      // ITERATION 2: Filter XYZ samples
      // -------------------------------------------------
      std::vector< std::vector< double > > filteredValues( _jointSmoothers.size() );
      for ( int i = 0; i < (int)_jointSmoothers.size(); ++i )
      {
         // Apply our filter across all the values we grabbed.
         // Notice rangeStart and actualNumValues will be set multiple times, which is okay since values remain constant
         // across all joints.
         rangeStart = _jointSmoothers[i]->Apply( filteredValues[i], flush );
         rangeEnd = rangeStart + (int)filteredValues[i].size();
      }

      // -------------------------------------------------
      // ITERATION 3: Create new poses from filtered values
      // -------------------------------------------------
      std::map< int, std::unique_ptr< Pose > > filteredFrames;
      for ( int frameIndex = 0; frameIndex < (int)filteredValues.back().size(); ++frameIndex )
      {
         // Get the unfiltered rig
         auto & unfilteredPose = _frames.at( actualRangeStart + frameIndex );
         
         // Create a new Pose with our adjusted timestamp and a copy of the original data.
         Pose * filteredPose = unfilteredPose->Clone();
         filteredPose->Timestamp( rangeStart + frameIndex );

         // For every joint in the rig
         for ( int i = 0; i < (int)filteredValues.size() / 3; ++i )
         {
            std::array< double, 3 > value =
            {
               filteredValues[ i * 3 + 0 ][ frameIndex ],
               filteredValues[ i * 3 + 1 ][ frameIndex ],
               filteredValues[ i * 3 + 2 ][ frameIndex ]
            };
            
            // Set our filtered value on each rotation component
            filteredPose->Keypoint( value, i );
         }
   
         // Generate a rig since this filtered frame only has XYZ data
         filteredPose->GenerateRig();
         
         // Add this rig frame to our filtered frames map
         filteredFrames.emplace( std::make_pair( filteredPose->Timestamp(), std::move(filteredPose) ) );
      }
      
      // Now replace our unfiltered frames with filtered frames
      _frames = std::move( filteredFrames );
   }
}
void AnimatedRig::SmoothAllBoneRolls( SMOOTH_TYPE type,
   int rangeStart,
   int rangeEnd,
   bool flush )
{
   // Derived from data analysis, this is the smooth factor
   const double NORMALIZED_FREQUENCY = 1./10.;

   int actualRangeStart = rangeEnd + 1;
   int actualRangeEnd = rangeStart - 1;
   
   // Sanity check
   if ( !_frames.size() )
      return;
      
   // We only care about joints with more than 1 degree of freedom:
   //   - hip roll (where the knee points)
   //   - shoulder roll (where the elbow points)
   const std::vector< Rig::JOINT_TYPE > jointsWithRoll =
   {
      Rig::LHIP,
      Rig::RHIP,
      Rig::LSHOULDER,
      Rig::RSHOULDER
   };

   // -------------------------------------------------
   // ITERATION 1: Add bone roll samples from every frame
   //
   // -------------------------------------------------
   for ( const auto & frame : _frames )
   {
      auto timestamp = frame.first;
      if ( timestamp >= rangeStart &&
         timestamp <= rangeEnd )
      {
         // Keep track of actual timestamps
         if ( timestamp < actualRangeStart )
            actualRangeStart = timestamp;
         if ( timestamp > actualRangeEnd )
            actualRangeEnd = timestamp;
            
         // Get the rig for this pose
         const Rig & rig = frame.second->RigPose().GetRig();

         // For every roll value in the specified joints
         for ( size_t i = 0; i < jointsWithRoll.size(); ++i )
         {
            // Lazy initialization of an LPF object
            if ( i >= _boneRollSmoothers.size() )
            {
               auto filter = SmoothFactory::Create( type );
               filter->Initialize( NUM_TAPS, NORMALIZED_FREQUENCY );
               _boneRollSmoothers.emplace_back( std::move(filter) );
            }
            _boneRollSmoothers[ i ]->AddSample( timestamp, rig.GetJoint( jointsWithRoll[ i ] ).roll );
         }
      }
   }

   if ( actualRangeEnd - actualRangeStart > 0 )
   {
      // Determine how much output space we need for filtered values, accounting for additional frames if flushing.
      // Also account for time shift.
      size_t expectedNumValues = actualRangeEnd - actualRangeStart + 1;
      //size_t actualNumValues = 0;
      rangeEnd -= _boneRollSmoothers[ 0 ]->GetSampleShift();
      if ( flush )
      {
         expectedNumValues += _jointSmoothers[ 0 ]->GetSampleShift();
         rangeEnd += _boneRollSmoothers[ 0 ]->GetSampleShift();
      }

      // -------------------------------------------------
      // ITERATION 2: Filter samples
      // -------------------------------------------------
      std::vector< std::vector< double > > filteredValues( _boneRollSmoothers.size() );
      for ( int i = 0; i < (int)_boneRollSmoothers.size(); ++i )
      {
         // Apply our filter across all the values we grabbed.
         // Notice rangeStart and actualNumValues will be set multiple times, which is okay since values remain constant
         // across all joints.
         rangeStart = _boneRollSmoothers[i]->Apply( filteredValues[i], flush );
         rangeEnd = rangeStart + (int)filteredValues[i].size();
      }

      // -------------------------------------------------
      // ITERATION 3: Apply the filtered roll
      // -------------------------------------------------
      for ( int frameIndex = 0; frameIndex < (int)filteredValues.back().size(); ++frameIndex )
      {
         // Get the rig
         auto & rig = _frames.at( actualRangeStart + frameIndex )->RigPose().GetRig();

         // For every specific joint in the rig
         for ( int i = 0; i < (int)filteredValues.size(); ++i )
         {
            // Get the joint
            Joint & joint = rig.GetJoint( jointsWithRoll[ i ] );
            
            // Get the original rotation
            Eigen::Quaterniond rotation( Utility::RawToQuaternion( joint.quaternion ) );
            
            // Un-apply the roll, since it's baked into the original rotation
            rotation = rotation * Eigen::AngleAxisd( joint.roll, Eigen::Vector3d::UnitY() ).inverse();
            
            // Now apply the filtered roll
            joint.roll = filteredValues[ i ][ frameIndex ];
            rotation = rotation * Eigen::AngleAxisd( joint.roll, Eigen::Vector3d::UnitY() );
            joint.quaternion = Utility::QuaternionToRaw( rotation );
         }
      }
   }
}
int AnimatedRig::Write( nlohmann::json & json,
   int startTimestamp,
   int endTimestamp )
{
   int numJointRotationsPerFrame = 0, numJointOffsetsPerFrame = 0;
   int lastTimestamp = startTimestamp;
   
   // Determine the maximum number of frames we can have
   size_t maxNumFrames = endTimestamp - startTimestamp + 1;
   
   // Vectors will grow as needed   
   std::vector< double > locations;
   std::vector< double > lengths;
   std::vector< double > rotations;
   std::vector< double > offsets;
   
   // Allocate a big buffer for the compressed data
   std::vector< uint8_t > buffer1( maxNumFrames * Rig::MAX_ROTATIONS_DIMENSION * sizeof(double) );
   
   // For every frame in this animated character
   auto it = _frames.begin();
   while ( it != _frames.end() )
   {
      // Get the pose
      auto & pose = (*it).second;
      
      // If this frame is within our bounds
      if ( pose->Timestamp() >= startTimestamp &&
         pose->Timestamp() <= endTimestamp )
      {
         lastTimestamp = pose->Timestamp();
         
         // Reset the bone lengths since they do not change between frames,
         // we only need one set
         lengths.clear();

         // Set average bone lengths
         UpdateBoneLengths( pose );
         
         // Generate the final rig if we haven't already (expecting it to check internally)
         pose->GenerateRig();
         
         // Update internal values if needed
         if ( numJointRotationsPerFrame == 0 )
            numJointRotationsPerFrame = pose->RigPose().GetRig().numJointsUsed;
         if ( numJointOffsetsPerFrame == 0 )
            numJointOffsetsPerFrame = pose->RigPose().GetRig().numJointOffsetsUsed;
         
         // Get the rig and appended values as arrays to any previous poses
         pose->RigPose().GetRig().ToArrays( locations,
            lengths,
            rotations,
            offsets );

         // Remove this frame
         it = _frames.erase( it );
      }
      else
      {
         // If this frame is outdated and no longer needed
         if ( pose->Timestamp() < startTimestamp )
         {
            // Remove it
            it = _frames.erase( it );
         }
         else
         {
            // We've passed the end, all done
            break;
         }
      }
   }
   
   std::vector< unsigned char > base64Data;
   
   // Compress the position data, encode to base64, and set to json
   if ( locations.size() )
   {
      size_t bufferSize = locations.size() * sizeof(double);
      Compression::EncodeZfp( &locations[0],
         locations.size(),
         &buffer1[0],
         bufferSize );
      bufferSize = Compression::EncodeBase64( &buffer1[0],
         bufferSize,
         base64Data );
      json["loc"] = std::string( reinterpret_cast< char * >(&base64Data[0]), bufferSize );
   }

   if ( lengths.size() )
   {
      // Same for bone lengths
      size_t bufferSize = lengths.size() * sizeof(double);
      Compression::EncodeZfp( &lengths[0],
         lengths.size(),
         &buffer1[0],
         bufferSize );
      bufferSize = Compression::EncodeBase64( &buffer1[0],
         bufferSize,
         base64Data );
      json["boneLen"] = std::string( reinterpret_cast< char * >(&base64Data[0]), bufferSize );
      json["numLen"] = lengths.size();
   }

   if ( rotations.size() )
   {
      // Same for bone rotations
      size_t bufferSize = rotations.size() * sizeof(double);
      Compression::EncodeZfp( &rotations[0],
         rotations.size(),
         &buffer1[0],
         bufferSize );
      bufferSize = Compression::EncodeBase64( &buffer1[0],
         bufferSize,
         base64Data );
      json["boneRot"] = std::string( reinterpret_cast< char * >(&base64Data[0]), bufferSize );
      json["numRot"] = numJointRotationsPerFrame;
   }

   if ( offsets.size() )
   {
      // Same for bone offsets
      size_t bufferSize = offsets.size() * sizeof(double);
      Compression::EncodeZfp( &offsets[0],
         offsets.size(),
         &buffer1[0],
         bufferSize );
      bufferSize = Compression::EncodeBase64( &buffer1[0],
         bufferSize,
         base64Data );
      json["boneOff"] = std::string( reinterpret_cast< char * >(&base64Data[0]), bufferSize );
      json["numOff"] = numJointOffsetsPerFrame;
   }
   
   return lastTimestamp;
}

