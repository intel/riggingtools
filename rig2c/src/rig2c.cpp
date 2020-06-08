#include "rig2c.h"
#include <fstream>
#include <thread>
#include <mutex>
#include <sstream>
#include <algorithm>
#include <string.h>
#include <json.hpp>
#include "Compression.hpp"
#include "Rig.hpp"
#include "Utility.hpp"
#include "config.h"

// Global static variables
API_TYPE_NAME( RETURN_CODE ) g_lastError = API_TYPE_NAME( API_NOT_INITIALIZED );
OnErrorDelegate g_errorDelegate = nullptr;
OnBoundsDelegate g_boundsDelegate = nullptr;
OnFrameDelegate g_frameDelegate = nullptr;
void * g_platformContext = nullptr;
bool g_stopReading = false;
std::thread g_readThread;
nlohmann::json g_json;
std::string g_currentJsonFilename;

API_TYPE_NAME( RETURN_CODE ) CheckVersion( std::string version )
{
   API_TYPE_NAME( RETURN_CODE ) returnValue = API_TYPE_NAME( NO_ERROR );

   // Parse the version string
   std::vector< int > versionComponents;
   std::stringstream versionSS( version );
   std::string token;
   while ( getline( versionSS, token, '.' ) )
   {
      int component = 0;
      try
      {
         component = std::stoi( token );
      }
      catch ( ... ) {}
      versionComponents.push_back( component );
   }

   // Make sure the major is exact and minor is at least as high
   if ( versionComponents[ 0 ] != std::stoi( MY_VERSION_MAJOR ) )
      returnValue = API_TYPE_NAME( BAD_FILE_VERSION ); 
   else if ( versionComponents[ 1 ] < std::stoi( MY_VERSION_MINOR ) )
      returnValue = API_TYPE_NAME( BAD_FILE_VERSION );

   return returnValue;
}

API_TYPE_NAME( RETURN_CODE ) ReadJson( std::string jsonFilename )
{
  static std::mutex lock;
  std::lock_guard< std::mutex > autoLock( lock );
   
   // Don't re-read the same file, if file names are same and g_json is not empty
   if ( g_currentJsonFilename == jsonFilename && !g_json.empty() )
      return API_TYPE_NAME( NO_ERROR );
   
   // Reset error
   g_lastError = API_TYPE_NAME( NO_ERROR );
   
   // Open the file
   std::ifstream i( jsonFilename );
   if ( !i.good() )
   {
      g_lastError = API_TYPE_NAME( BAD_PATH );
      if ( g_errorDelegate )
      {
         std::stringstream ss;
         char buffer[512];
         buffer[0] = 0;
         auto dontCare = strerror_s( buffer, sizeof( buffer ), errno ); (void)dontCare;
         ss << "Could not open '" << jsonFilename << "': " << buffer;
         g_errorDelegate( "", g_lastError, ss.str().c_str() );
      }
      return g_lastError;
   }
   
   // Parse the g_json file using overloaded istream-like operator
   try
   {
      i >> g_json;
   }
   // For when we move to v3: catch ( nlohmann::json::parse_error e )
   catch ( std::invalid_argument e )
   {
      g_lastError = API_TYPE_NAME( BAD_FILE_DATA );
      if ( g_errorDelegate )
      {
         std::stringstream ss;
         ss << "Could not parse '" << jsonFilename << "': " << e.what();
         g_errorDelegate( "", g_lastError, ss.str().c_str() );
      }
      return g_lastError;
   }
   catch ( ... )
   {
      g_lastError = API_TYPE_NAME( BAD_FILE_DATA );
      if ( g_errorDelegate )
         g_errorDelegate( "", g_lastError, "Could not load JSON file" );
      return g_lastError;
   }
   
   g_currentJsonFilename = jsonFilename;
   
   return g_lastError;
}

API_TYPE_NAME( RETURN_CODE ) API_CALLING_CONVENTION API_FUNC_NAME( initialize )( API_ARG_PREFIX
   API_TYPE_NAME( VOID_PTR ) platformContext )
{
   // Reset static variables
   g_lastError = API_TYPE_NAME( NO_ERROR );
   g_errorDelegate = nullptr;
   g_boundsDelegate = nullptr;
   g_frameDelegate = nullptr;
   g_stopReading = false;
   g_json.clear();
   
   g_platformContext = platformContext;

   return g_lastError;
}
API_TYPE_NAME( VOID ) API_CALLING_CONVENTION API_FUNC_NAME( uninitialize )( API_ARG_NONE )
{
   // Call stop as a good measure
   API_FUNC_NAME(stopRead)();
   
   g_lastError = API_TYPE_NAME( API_NOT_INITIALIZED );
}
API_TYPE_NAME( VOID ) API_CALLING_CONVENTION API_FUNC_NAME( setErrorCallback )( API_ARG_PREFIX OnErrorDelegate delegate )
{
   g_errorDelegate = delegate;
}
API_TYPE_NAME( VOID ) API_CALLING_CONVENTION API_FUNC_NAME( setBoundsCallback )( API_ARG_PREFIX OnBoundsDelegate delegate )
{
   g_boundsDelegate = delegate;
}
API_TYPE_NAME( VOID ) API_CALLING_CONVENTION API_FUNC_NAME( setFrameCallback )( API_ARG_PREFIX OnFrameDelegate delegate )
{
   g_frameDelegate = delegate;
}
API_TYPE_NAME( VOID_PTR ) API_CALLING_CONVENTION API_FUNC_NAME( getPlatformContext )( API_ARG_NONE )
{
   return g_platformContext;
}
API_TYPE_NAME( RETURN_CODE ) API_CALLING_CONVENTION API_FUNC_NAME( getLastError )( API_ARG_NONE )
{
   return g_lastError;
}
API_TYPE_NAME( RETURN_CODE ) API_CALLING_CONVENTION API_FUNC_NAME( getRestPoseHumanoid )( API_ARG_PREFIX
   API_TYPE_NAME( STRING ) previousName,
   API_TYPE_NAME( JOINT_REF ) joint )
{
   if ( g_lastError == API_TYPE_NAME( API_NOT_INITIALIZED ) )
      return g_lastError;
      
   g_lastError = API_TYPE_NAME( NO_ERROR );
   
   // We only need to get the default humanoid pose once
   static Rig restPose = Rig::RestPoseHumanoid();
   Joint restPoseJoint;
   size_t jointIndex = 0;
   Rig::JOINT_TYPE jointType = Rig::GetJointType( previousName ? std::string( previousName ) : "" );
   
   // If previousName is null or empty
   if ( previousName == nullptr || previousName[ 0 ] == 0 )
   {
      restPoseJoint = restPose.pelvis;
   }
   else
   {
      // Move to the next joint. We accomplish this by getting the previous joint type
      // and adding 1 because each joint is a member of a contiguous enumeration.
      // Return NO_MORE_DATA if we reach the end
      jointIndex = size_t(jointType) + 1;
      if ( jointIndex >= Rig::MAX_NUM_JOINTS )
         return API_TYPE_NAME( NO_MORE_DATA );
      jointType = Rig::JOINT_TYPE( jointIndex );
      restPoseJoint = restPose.GetJoint( jointType );
   }
   
   // Fill the name
   joint->name = Rig::GetJointType( jointType );
   
   // Fill the parent name
   joint->parentName = Rig::GetJointType( Rig::GetJointParent( jointType ) );
   
   // Set the offset, if any
   joint->offsetX = restPoseJoint.offset[0];
   joint->offsetY = restPoseJoint.offset[1];
   joint->offsetZ = restPoseJoint.offset[2];
   
   // Set the rotation
   joint->quaternionX = restPoseJoint.quaternion[0];
   joint->quaternionY = restPoseJoint.quaternion[1];
   joint->quaternionZ = restPoseJoint.quaternion[2];
   joint->quaternionW = restPoseJoint.quaternion[3];
   
   // Set the length
   joint->length = restPoseJoint.length;
   
   // Set the index
   joint->index = API_TYPE_NAME( INT )(jointIndex);
   
   return API_TYPE_NAME( NO_ERROR );
}
API_TYPE_NAME( RETURN_CODE ) API_CALLING_CONVENTION API_FUNC_NAME( getInfo )( API_ARG_PREFIX
   API_TYPE_NAME( STRING ) url,
   API_TYPE_NAME( STRING ) key,
   API_TYPE_NAME( STRING_REF ) value,
   API_TYPE_NAME( INT ) valueSize )
{
   if ( g_lastError == API_TYPE_NAME( API_NOT_INITIALIZED ) )
      return g_lastError;
   
   // Open a new file ONLY if:
   //  - We don't already have a file open
   //  - The url has something possibly valid
   std::string jsonFilename = Utility::ExpandTilde( std::string( url ) );
   if ( g_json.empty() && jsonFilename != "" )
   {
      // Try and read the JSON file
      std::string jsonFilename = Utility::ExpandTilde( std::string( url ) );
      ReadJson( jsonFilename );
      if ( g_lastError != API_TYPE_NAME( NO_ERROR ) )
         return g_lastError;
   }
   
   if ( !g_json.empty() )
   {
      // Handle "version" special, since it isn't part of the header
      if ( strcmp( key, "version" ) == 0 )
      {
         std::stringstream ss;
         ss << g_json["version"];
         int length = std::min( (int)ss.str().length(), valueSize - 1 );
         memcpy( value, ss.str().c_str(), length );
         value[ length ] = 0;
      }
      else
      {
         // VERSION CHECK!!!
         g_lastError = CheckVersion( g_json["version"] );
         if ( API_TYPE_NAME( NO_ERROR ) != g_lastError )
            return g_lastError;
      
         auto idIt = g_json["header"].find( key );
         if ( idIt != g_json["header"].end() )
         {
            std::stringstream ss;
            ss << (*idIt);
            int length = std::min( (int)ss.str().length(), valueSize - 1 );
            memcpy( value, ss.str().c_str(), length );
            value[ length ] = 0;
         }
      }
   }
   else
   {
      return API_TYPE_NAME( NO_FILE_LOADED );
   }

   return API_TYPE_NAME( NO_ERROR );
}
API_TYPE_NAME( RETURN_CODE ) API_CALLING_CONVENTION API_FUNC_NAME( getRigInfo )( API_ARG_PREFIX
   API_TYPE_NAME( STRING ) url,
   API_TYPE_NAME( STRING ) rigId,
   API_TYPE_NAME( STRING ) key,
   API_TYPE_NAME( STRING_REF ) value,
   API_TYPE_NAME( INT ) valueSize )
{
   if ( g_lastError == API_TYPE_NAME( API_NOT_INITIALIZED ) )
      return g_lastError;
   
   // Open a new file ONLY if:
   //  - We don't already have a file open
   //  - The url has something possibly valid
   std::string jsonFilename = Utility::ExpandTilde( std::string( url ) );
   if ( g_json.empty() && jsonFilename != "" )
   {
      // Try and read the JSON file
      ReadJson( jsonFilename );
      if ( g_lastError != API_TYPE_NAME( NO_ERROR ) )
         return g_lastError;
   }
   
   if ( !g_json.empty() )
   {
      // VERSION CHECK!!!
      g_lastError = CheckVersion( g_json["version"] );
      if ( API_TYPE_NAME( NO_ERROR ) != g_lastError )
         return g_lastError;
      
      for ( nlohmann::json::const_iterator it = g_json["rigs"].begin(); it != g_json["rigs"].end(); ++it )
      {
         auto idIt = (*it).find( "id" );
         if ( idIt != (*it).end() && (*idIt).get<std::string>() == std::string(rigId) )
         {
            auto keyIt = (*it).find( key );
            if (keyIt != (*it).end())
            {
               std::string jsonValue = (*keyIt).get<std::string>();
               int length = std::min( (int)jsonValue.length(), valueSize - 1 );
               memcpy( value, jsonValue.c_str(), length );
               value[ length ] = 0;
            }               
            break;
         }
      }
   }
   else
   {
      return API_TYPE_NAME( NO_FILE_LOADED );
   }

   return API_TYPE_NAME( NO_ERROR );
}
API_TYPE_NAME( RETURN_CODE ) API_CALLING_CONVENTION API_FUNC_NAME( startRead )( API_ARG_PREFIX
   API_TYPE_NAME( STRING ) url )
{
   if ( g_lastError == API_TYPE_NAME( API_NOT_INITIALIZED ) )
      return g_lastError;
   
   std::string urlCopy( url );
   
   API_TYPE_NAME( RETURN_CODE ) returnValue = API_TYPE_NAME( NO_ERROR );
   g_readThread = std::thread( [&returnValue, urlCopy]
   {
         returnValue = API_FUNC_NAME( read )( urlCopy.c_str() );
   });
   
   return returnValue;
}
API_TYPE_NAME( RETURN_CODE ) API_CALLING_CONVENTION API_FUNC_NAME( read )( API_ARG_PREFIX
   API_TYPE_NAME( STRING ) url )
{
   if ( g_lastError == API_TYPE_NAME( API_NOT_INITIALIZED ) )
      return g_lastError;
      
   // We require at least on of these callbacks or what's the point?
   if ( g_frameDelegate == nullptr && g_boundsDelegate == nullptr )
   {
      return API_TYPE_NAME( NO_CALLBACK );
   }
   
   // Reset error
   g_lastError = API_TYPE_NAME( NO_ERROR );
   
   // Get local references
   // TODO: Not sure we need this
   bool & stopReading = g_stopReading;
   API_TYPE_NAME( RETURN_CODE ) & lastError = g_lastError;
   OnErrorDelegate errorDelegate = g_errorDelegate;
   OnBoundsDelegate boundsDelegate = g_boundsDelegate;
   OnFrameDelegate frameDelegate = g_frameDelegate;

   // Try and read the JSON file
   std::string jsonFilename = Utility::ExpandTilde( std::string( url ? url : "" ) );
   ReadJson( jsonFilename );
   if ( g_lastError != API_TYPE_NAME( NO_ERROR ) )
      return g_lastError;
      
   // VERSION CHECK!!!
   g_lastError = CheckVersion( g_json["version"] );
   if ( API_TYPE_NAME( NO_ERROR ) != g_lastError )
      return g_lastError;
   
   nlohmann::json::const_iterator it = g_json["rigs"].begin();
   if ( it == g_json["rigs"].end() )
   {
      lastError = API_TYPE_NAME( BAD_FILE_DATA );
      if ( errorDelegate )
         errorDelegate( "", lastError, "No rigs defined in JSON file" );
      return lastError;
   }

   // For each rig
   for ( ; it != g_json["rigs"].end(); ++it )
   {
      if ( stopReading )
         break;
         
      // Get the character name
      std::string rigId = (*it)["name"].get<std::string>();
      
      // Determine character bounds
      int startFrame = (int)g_json["header"]["startFrame"];
      int endFrame = (int)g_json["header"]["endFrame"];
      if ( (*it).find("startFrame") != (*it).end() )
         startFrame = (int)(*it)["startFrame"];
      if ( (*it).find("endFrame") != (*it).end() )
         endFrame = (int)(*it)["endFrame"];
      
      // Make the bounds callback
      if ( boundsDelegate )
         boundsDelegate( rigId.c_str(),
            startFrame,
            endFrame );

      size_t dataSize = 0;
      std::vector< unsigned char > base64Data;
      std::vector< double > positions;
      std::vector< double > lengths;
      std::vector< double > rotations;
      std::vector< double > offsets;
      
      int numLengthsPerFrame = 0;
      int numRotationsPerFrame = 0;
      int numOffsetsPerFrame = 0;
      
      // Decode locations
      dataSize = (*it)["loc"].get<std::string>().size();
      dataSize = Compression::DecodeBase64( (const unsigned char *)&(*it)["loc"].get_ref<const nlohmann::json::string_t&>()[0],
         dataSize,
         base64Data );
      Compression::DecodeZfp( &base64Data[0],
         dataSize,
         positions );

      // If we have lengths
      auto arrayIt = (*it).find("boneLen");
      auto lengthIt = (*it).find("numLen");
      if ( arrayIt != (*it).end() &&
         lengthIt != (*it).end() &&
         (numLengthsPerFrame = (*lengthIt).get<int>()) > 0 )
      {
         // Grab 'em
         dataSize = (*it)["boneLen"].get<std::string>().size();
         dataSize = Compression::DecodeBase64( (const unsigned char *)&(*it)["boneLen"].get_ref<const nlohmann::json::string_t&>()[0],
            dataSize,
            base64Data );
         Compression::DecodeZfp( &base64Data[0],
            dataSize,
            lengths );
         
         // Validate
         if ( (int)lengths.size() < numLengthsPerFrame )
         {
            lastError = API_TYPE_NAME( BAD_FILE_DATA );
            if ( errorDelegate )
            {
               std::stringstream ss;
               ss << "Decoded lengths size (" << lengths.size() << ") didn't match numLengthsPerFrame (" << numLengthsPerFrame << ") in "<< jsonFilename;
               errorDelegate( "", lastError, ss.str().c_str() );
            }
            return lastError;
         }
      }
      
      // If we have rotations
      arrayIt = (*it).find("boneRot");
      lengthIt = (*it).find("numRot");
      if ( arrayIt != (*it).end() &&
         lengthIt != (*it).end() &&
         (numRotationsPerFrame = (*lengthIt).get<int>()) > 0 )
      {
         // Grab 'em
         dataSize = (*it)["boneRot"].get<std::string>().size();
         dataSize = Compression::DecodeBase64( (const unsigned char *)&(*it)["boneRot"].get_ref<const nlohmann::json::string_t&>()[0],
            dataSize,
            base64Data );
         Compression::DecodeZfp( &base64Data[0],
            dataSize,
            rotations );
         
         // Validate
         if ( (int)rotations.size() < (numRotationsPerFrame * 4) )
         {
            lastError = API_TYPE_NAME( BAD_FILE_DATA );
            if ( errorDelegate )
            {
               std::stringstream ss;
               ss << "Decoded rotations size (" << dataSize << ") didn't match numRotationsPerFrame*4 (" << numRotationsPerFrame*4 << ") in "<< jsonFilename;
               errorDelegate( "", lastError, ss.str().c_str() );
            }
            return lastError;
         }
         
         // zfp is *approximate*, so sometimes quaternion components are outside the bounds [-1,1]
         // Take care of that here
         double * rotationsRaw = (double *)(&rotations[0]);
         for ( int i = 0; i < (int)rotations.size(); ++i )
            rotationsRaw[i] = CLIP(rotationsRaw[i]);
      }
      
      // If we have offsets
      arrayIt = (*it).find("boneOff");
      lengthIt = (*it).find("numOff");
      if ( arrayIt != (*it).end() &&
         lengthIt != (*it).end() &&
          (numOffsetsPerFrame = (*lengthIt).get<int>()) > 0 )
      {
         // Grab 'em
         dataSize = (*it)["boneOff"].get<std::string>().size();
         dataSize = Compression::DecodeBase64( (const unsigned char *)&(*it)["boneOff"].get_ref<const nlohmann::json::string_t&>()[0],
            dataSize,
            base64Data );
         Compression::DecodeZfp( &base64Data[0],
            dataSize,
            offsets );
         
         // Validate
         if ( (int)offsets.size() < (numOffsetsPerFrame * 3) )
         {
            lastError = API_TYPE_NAME( BAD_FILE_DATA );
            if ( errorDelegate )
            {
               std::stringstream ss;
               ss << "Decoded offsets data size (" << dataSize << ") didn't match numOffsetsPerFrame*3 (" << numOffsetsPerFrame*3 << ") in "<< jsonFilename;
               errorDelegate( "", lastError, ss.str().c_str() );
            }
            return lastError;
         }
      }
      
      // For each frame
      const int numFrames = endFrame - startFrame + 1;
      int counter = 0;
      while( !stopReading && counter < numFrames )
      {
         if ( frameDelegate )
         {
            frameDelegate( rigId.c_str(),
               startFrame + counter,
               &positions[ counter * Rig::LOCATION_DIMENSION ],
               numRotationsPerFrame ? &rotations[ counter * numRotationsPerFrame * 4 ] : nullptr,
               numRotationsPerFrame,
               numLengthsPerFrame ? &lengths[ 0 ] : nullptr,
               numLengthsPerFrame,
               numOffsetsPerFrame ? &offsets[ counter * numOffsetsPerFrame * 3 ] : nullptr,
               numOffsetsPerFrame );
         }
   
         ++counter;
      }
   }
   
   return API_TYPE_NAME( NO_ERROR );
}
API_TYPE_NAME( VOID ) API_CALLING_CONVENTION API_FUNC_NAME( stopRead )( API_ARG_NONE )
{
   g_stopReading = false;
   if ( g_readThread.joinable() )
      g_readThread.join();
}
