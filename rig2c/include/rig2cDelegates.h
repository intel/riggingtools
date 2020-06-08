/* Helper types for importing the shared object in C or C++ code. All functions have "Delegate" appended to
   the function name; for example, "rig_initialize" has function pointer typedef "rig_initializeDelegate".
   
Example:
   Assume the shared object is loaded (with dlopen() for example) and api functions have api-prefix "rig_".
   
   To get the pointer to the initialize function:
      rig_initializeDelegate initializeFunction = (rig_initializeDelegate)dlsym( dlHandle, "rig_initialize" );
      
   Then call the function:
      (*initializeFunction)( NULL );
*/

#include "rig2cTypes.h"

typedef API_TYPE_NAME( RETURN_CODE ) (*API_FUNC_NAME( initializeDelegate ))( API_ARG_PREFIX
   API_TYPE_NAME( VOID_PTR ) platformContext );
typedef API_TYPE_NAME( VOID ) (*API_FUNC_NAME( uninitializeDelegate ))( API_ARG_NONE );
typedef API_TYPE_NAME( VOID ) (*API_FUNC_NAME( setErrorCallbackDelegate ))( API_ARG_PREFIX
   OnErrorDelegate delegate );
typedef API_TYPE_NAME( VOID ) (*API_FUNC_NAME( setBoundsCallbackDelegate ))( API_ARG_PREFIX
   OnBoundsDelegate delegate );
typedef API_TYPE_NAME( VOID ) (*API_FUNC_NAME( setFrameCallbackDelegate ))( API_ARG_PREFIX
   OnFrameDelegate delegate );
typedef API_TYPE_NAME( VOID_PTR ) (*API_FUNC_NAME( getPlatformContextDelegate ))( API_ARG_NONE );
typedef API_TYPE_NAME( RETURN_CODE ) (*API_FUNC_NAME( getLastErrorDelegate ))( API_ARG_NONE );
typedef API_TYPE_NAME( RETURN_CODE ) (*API_FUNC_NAME( getInfoDelegate ))( API_ARG_PREFIX
   API_TYPE_NAME( STRING ) url,
   API_TYPE_NAME( STRING ) key,
   API_TYPE_NAME(STRING_REF) value,
   API_TYPE_NAME(INT) valueSize );
typedef API_TYPE_NAME( RETURN_CODE ) (*API_FUNC_NAME( getRigInfoDelegate ))( API_ARG_PREFIX
   API_TYPE_NAME( STRING ) url,
   API_TYPE_NAME( STRING ) rigId,
   API_TYPE_NAME( STRING ) key,
   API_TYPE_NAME( STRING_REF ) value,
   API_TYPE_NAME( INT ) valueSize );
typedef API_TYPE_NAME( RETURN_CODE ) (*API_FUNC_NAME( readDelegate ))( API_ARG_PREFIX
   API_TYPE_NAME( STRING ) url );
typedef API_TYPE_NAME( RETURN_CODE ) (*API_FUNC_NAME( startReadDelegate ))( API_ARG_PREFIX
   API_TYPE_NAME( STRING ) url );
typedef API_TYPE_NAME( VOID ) (*API_FUNC_NAME( stopReadDelegate ))( API_ARG_NONE );
