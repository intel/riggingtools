#include "rig2cTypes.h"

#ifdef __cplusplus
extern "C"{
#endif

   /**************************************************** SYSTEM ***********************************************************/
   /* Called once before calling any lib functions. Creates any persistent data and prepares the lib for use.
      Multiple calls are okay but ignored after the first.
    
   Inputs:
      in_platformContext: platform-specific context. MUST be the native activity if ANDROID, otherwise can be anything
      you like.
    
   Return value: return code */
   API_FUNC_DECLARE( API_TYPE_NAME( RETURN_CODE ) ) API_FUNC_NAME( initialize )( API_ARG_PREFIX
      API_TYPE_NAME( VOID_PTR ) platformContext );
   
   /* Called once before shutting down the application. Frees any persistent data stored in the lib and
      clears the directory cache. Multiple calls okay but ignored after the first.
    
   Return value: none */
   API_FUNC_DECLARE( API_TYPE_NAME( VOID ) ) API_FUNC_NAME( uninitialize )( API_ARG_NONE );
   
   /* Sets a callback when an error happens. This is useful for getting more detailed error information than just an
      error code. This does not change any error state and has no effect on getLastError().

   Inputs:
      delegate: the callback. Signature (in C) is OnErrorDelegate:
         void(*)( int rigId, int errorCode, const char * nullTerminatedDescription ) */
   API_FUNC_DECLARE( API_TYPE_NAME( VOID ) ) API_FUNC_NAME( setErrorCallback )( API_ARG_PREFIX
      OnErrorDelegate delegate );

   /* Sets a callback to receive skeletal frame bounds, whenever they change. Ensure you don't spend much time
      in this callback as it will block an internal worker thread.

   Inputs:
      delegate: the callback. Signature (in C) is OnBoundsDelegate:
         void(*)( int rigId, int startFrameNumber, int endFrameNumber ) */
   API_FUNC_DECLARE( API_TYPE_NAME( VOID ) ) API_FUNC_NAME( setBoundsCallback )( API_ARG_PREFIX
      OnBoundsDelegate delegate );
      
   /* Sets a callback to receive per-frame skeletal data. This callback is made from an internal thread.
   
   Inputs:
      delegate: the callback. Signature (in C) is OnFrameDelegate:
         void(*)( int rigId,
            int frameTimestamp,
            const double * locationXYZ,
            const double * boneRotations,
            int numBoneRotations,
            const double * boneLengths,
            int numBoneLengths,
            const double * boneOffsets,
            int numBoneOffsets,
            double unitMeters );
            
      locationXYZ is always 3 xyz values.
      numBoneRotations represents 1/4 the array size (number of doubles) in boneRotations, since each rotation is comprised of 4 xyzw values.
      numBoneLengths represents the array size (number of doubles) in boneLengths.
      numBoneOffsets represents 1/3 the array size (number of doubles) in boneOffsets, since each rotation is comprised of 3 xyz values.
   */
   API_FUNC_DECLARE( API_TYPE_NAME( VOID ) ) API_FUNC_NAME( setFrameCallback )( API_ARG_PREFIX
      OnFrameDelegate delegate );
      
   /* Returns the pointer supplied during the call to initialize().
       
      Return value: pointer to the context provided during initialize() */
   API_FUNC_DECLARE( API_TYPE_NAME( VOID_PTR ) ) API_FUNC_NAME( getPlatformContext )( API_ARG_NONE );
   
   /* Gets metadata for an entire segment as a string value. You are required to provice string
      buffer 'value' of size 'valueSize'. If no error occurs, 'value' will be updated with
      a guaranteed null-terminated string.
      
      'url' can be an empty (first character zero) string buffer if read() or startRead() has already been called.
   
      Return value: most recent error code */
   API_FUNC_DECLARE( API_TYPE_NAME( RETURN_CODE ) ) API_FUNC_NAME( getInfo )( API_ARG_PREFIX
      API_TYPE_NAME( STRING ) url,
      API_TYPE_NAME( STRING ) key,
      API_TYPE_NAME( STRING_REF ) value,
      API_TYPE_NAME( INT ) valueSize );
      
   /* Gets metadata for a specified rig as a string value. You are required to provice string
      buffer 'value' of size 'valueSize'. If no error occurs, 'value' will be updated with
      a guaranteed null-terminated string.
      
      'url' can be an empty (first character zero) string buffer if read() or startRead() has already been called.
   
      Return value: most recent error code */
   API_FUNC_DECLARE( API_TYPE_NAME( RETURN_CODE ) ) API_FUNC_NAME( getRigInfo )( API_ARG_PREFIX
      API_TYPE_NAME( STRING ) url,
      API_TYPE_NAME( STRING ) rigId,
      API_TYPE_NAME( STRING ) key,
      API_TYPE_NAME( STRING_REF ) value,
      API_TYPE_NAME( INT ) valueSize );

   /* Gets the most recent error
   
      Return value: most recent error code */
   API_FUNC_DECLARE( API_TYPE_NAME( RETURN_CODE ) ) API_FUNC_NAME( getLastError )( API_ARG_NONE );
   
   /* Returns the default pose (also 'rest pose' or 'bind pose') for a single joint. Keep calling this
      to walk the joint hierarchy for every joint until it returns 'NO_MORE_DATA'.
      This is useful if building a T-Pose in animation software or finding absolute positions of joints.
      
      'previousName' should be empty or NULL for the first (root) joint, then pass in the name from the
      previous call for the next call.
      
      'joint' is a reference to an allocated structure that will be filled if the function exits normally.
      The JOINT struct will be filled with global character strings for some of it's fields; this means don't
      deallocate any of the struct's character strings with free() or delete[]().
   
      Return value: most recent error code */
   API_FUNC_DECLARE( API_TYPE_NAME( RETURN_CODE ) ) API_FUNC_NAME( getRestPoseHumanoid )( API_ARG_PREFIX
      API_TYPE_NAME( STRING ) previousName,
      API_TYPE_NAME( JOINT_REF ) joint );
   
   /* Read data from a JSON file. This will create a thread in the background and make callbacks (bounds, frame, etc.)
      from that thread. This function performs basic initialization then returns; practically speaking it is non-blocking.
      This means data will continue to flow in until input is exhausted or stopRead() is called.
      Returns zero if successful, non-zero otherwise. If an error occurs the return value will be the API error code.
      The recommended method of getting detailed error information is to use getLastError() or the OnErrorDelegate.
      
      If streaming is desired, this function may be called from a streaming library's frame callback.
      
      Requires either OnBoundsDelegate or OnFrameDelegate be set to valid functions.
   
   Inputs:
      url: url of the JSON manifest file */
   API_FUNC_DECLARE( API_TYPE_NAME( RETURN_CODE ) ) API_FUNC_NAME( startRead )( API_ARG_PREFIX
      API_TYPE_NAME( STRING ) url );
      
   /* Read data from a JSON file. This will make callbacks (bounds, frame, etc.) from the calling thread, and will block until
      all data is exhausted. Use only for file-based URLs!
      Returns zero if successful, non-zero otherwise. If an error occurs the return value will be the API error code.
      The recommended method of getting detailed error information is to use getLastError() or the OnErrorDelegate.
      
      Requires either OnBoundsDelegate or OnFrameDelegate be set to valid functions.
   
   Inputs:
      url: url of the JSON manifest file */
   API_FUNC_DECLARE( API_TYPE_NAME( RETURN_CODE ) ) API_FUNC_NAME( read )( API_ARG_PREFIX
      API_TYPE_NAME( STRING ) url );
   
   /* Stop processing data, stop making callbacks, and destroy the thread. Blocks until everything is complete.
   */
   API_FUNC_DECLARE( API_TYPE_NAME( VOID ) ) API_FUNC_NAME( stopRead )( API_ARG_NONE );

#if defined (__cplusplus)
}
#endif
