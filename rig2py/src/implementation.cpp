extern "C"
{
   #include "CInterface.h"
}
#include "Joint.h"
#include <fstream>
#include <json.hpp>
#include "Compression.hpp"
#include "Rig.hpp"
#include "rig2c.h"

#define API_RETURN_HANDLER( ERROR_CODE, CMD_IF_SUCCESS ) \
   switch ( ERROR_CODE ) \
   { \
      case rig_NO_ERROR: CMD_IF_SUCCESS; break; \
      case rig_BAD_FILE_VERSION: PyErr_SetString( PyExc_RuntimeError, "Outdated/invalid version in file" ); break; \
      case rig_BAD_PATH: PyErr_SetString( PyExc_RuntimeError, "Bad filename or path" ); break; \
      case rig_NO_FILE_LOADED: PyErr_SetString( PyExc_RuntimeError, "No file loaded. Has a file URL been specified?" ); break; \
      case rig_BAD_FILE_DATA: PyErr_SetString( PyExc_RuntimeError, "Corrupt or missing data in file" ); break; \
      case rig_API_NOT_INITIALIZED: PyErr_SetString( PyExc_RuntimeError, "API not initialized" ); break; \
      case rig_NO_CALLBACK: PyErr_SetString( PyExc_RuntimeError, "No callbacks registered" ); break; \
      default: PyErr_SetString( PyExc_RuntimeError, "Unknown API error" ); break; \
   }

void Initialize()
{
   PyErr_Clear();
   
   rig_initialize( NULL );
}
void OnError( const char * rigId,
   rig_RETURN_CODE returnCode,
   const char * nullTerminatedDescription )
{
   PyErr_Clear();
   
   if ( g_errorCallback )
   {
      // Make the bounds callback
      PyObject * arglist = Py_BuildValue( "(sis)",
         rigId,
         returnCode,
         nullTerminatedDescription );
      PyObject * result = PyEval_CallObject( g_errorCallback, arglist );
      if ( result )
         Py_DECREF( result );
      Py_DECREF( arglist );
   }
}
void OnBounds( const char * rigId,
   int startTimestamp,
   int endTimestamp )
{
   PyErr_Clear();
   
   if ( g_boundsCallback )
   {
      // Make the bounds callback
      PyObject * arglist = Py_BuildValue( "(sii)",
         rigId,
         startTimestamp,
         endTimestamp );
      PyObject * result = PyEval_CallObject( g_boundsCallback, arglist );
      if ( result )
         Py_DECREF( result );
      Py_DECREF( arglist );
   }
}
void OnFrame( const char * rigId,
   int frameTimestamp,
   const double * locationXYZ,
   const double * boneRotations,
   int numBoneRotations,
   const double * boneLengths,
   int numBoneLengths,
   const double * boneOffsets,
   int numBoneOffsets )
{
   PyErr_Clear();
   
   if ( g_frameCallback )
   {
      // Create python values for this frame
      PyObject * pyPosition = BuildPythonArray( locationXYZ, Rig::LOCATION_DIMENSION );
      PyObject * pyRotations = numBoneRotations ? BuildPythonArray( boneRotations, numBoneRotations * 4 ) : Py_None;
      
      // We only use one set of bone lengths for all frames
      PyObject * pyLengths = numBoneLengths ? BuildPythonArray( boneLengths, numBoneLengths ) : Py_None;
      PyObject * pyOffsets = numBoneOffsets ? BuildPythonArray( boneOffsets, numBoneOffsets * 3 ) : Py_None;
    
      // Build args, make the python callback, cleanup
      PyObject * arglist = Py_BuildValue( "(siOOOO)",
         rigId,
         frameTimestamp,
         pyPosition,
         pyLengths,
         pyRotations,
         pyOffsets );
      PyObject * result = PyEval_CallObject( g_frameCallback, arglist );
      if ( result )
         Py_DECREF( result );
      Py_DECREF( arglist );
   }
}
PyObject * rig2python_errorCallback( PyObject * py, PyObject * args )
{
   PyErr_Clear();
   
   PyObject * delegate;
   if ( !PyArg_ParseTuple( args, "O", &delegate ) )
      return NULL;
   if ( !PyCallable_Check( delegate ) )
   {
      PyErr_SetString( PyExc_TypeError, "Need a callable object" );
   }
   else
   {
      g_errorCallback = delegate;
   }
   
   (void)py;
   
   return Py_None;
}
PyObject * rig2python_boundsCallback( PyObject * py, PyObject * args )
{
   PyErr_Clear();
   
   PyObject * delegate;
   if ( !PyArg_ParseTuple( args, "O", &delegate ) )
      return NULL;
   if ( !PyCallable_Check( delegate ) )
   {
      PyErr_SetString( PyExc_TypeError, "Need a callable object" );
   }
   else
   {
      g_boundsCallback = delegate;
   }
   
   (void)py;
   
   return Py_None;
}
PyObject * rig2python_frameCallback( PyObject * py, PyObject * args )
{
   PyErr_Clear();
   
   PyObject * delegate;
   if ( !PyArg_ParseTuple( args, "O", &delegate ) )
      return NULL;
   if ( !PyCallable_Check( delegate ) )
   {
      PyErr_SetString( PyExc_TypeError, "Need a callable object" );
   }
   else
   {
      g_frameCallback = delegate;
   }
   (void)py;
   
   return Py_None;
}
PyObject * rig2python_getInfo( PyObject * py, PyObject * args )
{
   PyErr_Clear();
   
   PyObject* returnValue = nullptr;
   
   // Get the url and key
   const char * url;
   const char * key;
   if ( !PyArg_ParseTuple( args, "ss",
      &url,
      &key ) )
   {
      PyErr_SetString( PyExc_RuntimeError, "Could not parse arguments" );
   }
   else
   {
      char value[512];
      API_RETURN_HANDLER( rig_getInfo( url, key, value, sizeof(value) ), returnValue = Py_BuildValue( "s", value ) );
   }
   (void)py;
   
   return returnValue;
}
PyObject * rig2python_getRigInfo( PyObject * py, PyObject * args )
{
   PyErr_Clear();
   
   PyObject* returnValue = nullptr;
   
   // Get the url, id and key
   const char * url;
   const char * rigId;
   const char * key;
   if ( !PyArg_ParseTuple( args, "sss",
      &url,
      &rigId,
      &key ) )
   {
      PyErr_SetString( PyExc_RuntimeError, "Could not parse arguments" );
   }
   else
   {
      char value[512];
      API_RETURN_HANDLER( rig_getRigInfo( url, rigId, key, value, sizeof(value) ), returnValue = Py_BuildValue( "s", value ) );
   }

   (void)py;
   
   return returnValue;
}
PyObject * rig2python_getRestPoseHumanoid( PyObject * py, PyObject * args )
{
   PyErr_Clear();
   
   PyObject * returnValue = nullptr;
   
   // Get the url, id and key
   const char * previousName;
   if ( !PyArg_ParseTuple( args, "s",
      &previousName ) )
   {
      PyErr_SetString( PyExc_RuntimeError, "Could not parse arguments" );
   }
   else
   {
      rig_JOINT joint;
      
      rig_RETURN_CODE success = rig_getRestPoseHumanoid( previousName, &joint );
      switch ( success )
      {
         case rig_NO_MORE_DATA: return Py_None;
         default: API_RETURN_HANDLER( success, ; );
      }
      
      // Create a python-friendly joint object
      PyObject * argList = Py_BuildValue("ssOOdi",
         joint.name,
         joint.parentName,
         BuildPythonArray( &joint.offsetX, 3 ),
         BuildPythonArray( &joint.quaternionX, 4 ),
         joint.length,
         joint.index );
      returnValue = PyObject_CallObject( jointType, argList );
      Py_DECREF( argList );
   }

   (void)py;
   
   return returnValue;
}
PyObject * rig2python_read( PyObject * py, PyObject * args )
{
   PyErr_Clear();
   
   PyObject* returnValue = nullptr;
   
   // Get the filename
   const char * jsonFilename;
   if ( !PyArg_ParseTuple( args, "s", &jsonFilename ) )
      return NULL;
   
   rig_setErrorCallback( OnError );
   rig_setBoundsCallback( OnBounds );
   rig_setFrameCallback( OnFrame );
   
   API_RETURN_HANDLER( rig_read( jsonFilename ), returnValue = Py_None );
   
   (void)py;
   
   return returnValue;
}
