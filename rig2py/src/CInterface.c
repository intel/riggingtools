#include "CInterface.h"
#include "Joint.h"

PyObject * g_boundsCallback = NULL;
PyObject * g_frameCallback = NULL;
PyObject * g_errorCallback = NULL;

static int module_exec( PyObject * module )
{
   jointType = CreateJointType();
   if ( PyType_Ready( (PyTypeObject *)jointType ) < 0 )
   {
      return -1;
   }
    
   Py_INCREF( jointType );
   if ( PyModule_AddObject( module, "Joint", jointType ) < 0 )
   {
      Py_XDECREF( jointType );
      return -1;
   }
   
   return 0;
}

static struct PyModuleDef_Slot rig2python_slots[] =
{
   { Py_mod_exec, module_exec },
   { 0, NULL },
};

static PyMethodDef rig2python_functions[] =
{
   { "errorCallback", (PyCFunction)rig2python_errorCallback, METH_VARARGS, "Sets the callback function for when an error occurs" },
   { "boundsCallback", (PyCFunction)rig2python_boundsCallback, METH_VARARGS, "Sets the callback function for when the bounds change" },
   { "frameCallback", (PyCFunction)rig2python_frameCallback, METH_VARARGS, "Sets the callback function for when a new frame is ready" },
   { "getInfo", (PyCFunction)rig2python_getInfo, METH_VARARGS, "Gets global metadata by key" },
   { "getRigInfo", (PyCFunction)rig2python_getRigInfo, METH_VARARGS, "Gets metadata from a specific rigId and key" },
   { "getRestPoseHumanoid", (PyCFunction)rig2python_getRestPoseHumanoid, METH_VARARGS, "Returns the default pose (also 'rest pose' or 'bind pose') for a single joint.\
      Keep calling this to walk the joint hierarchy for every joint until it returns 'NO_MORE_DATA'." },
   { "read", (PyCFunction)rig2python_read, METH_VARARGS, "Reads all data from a json file" },
   { NULL, NULL, 0, NULL }
};

static struct PyModuleDef moduleDef =
{
   PyModuleDef_HEAD_INIT,
   Py_STRINGIFY(MODULE_NAME),
   "Reads and decodes json rig data to a python via callbacks",
   0, /* -1 for single-phase */
   rig2python_functions,
   rig2python_slots, /* NULL for single-phase */
   NULL,
   NULL,
   NULL
};

/* Python entry point, suffixed with the same as the module file it produces */
#define JOIN(a,b) JOIN_AGAIN(a,b)
#define JOIN_AGAIN(a,b) a ## b
PyMODINIT_FUNC JOIN(PyInit_,MODULE_NAME)(void)
{
   Initialize();

   /* Multi-phase initialization */
   return PyModuleDef_Init( &moduleDef );
}

// Helper for building floating point arrays
PyObject * BuildPythonArray( const double * array,
   size_t arraySize )
{
   PyObject * pyArray = PyList_New( arraySize );
   for ( int i = 0; i < (int)arraySize; ++i )
   {
       PyList_SetItem( pyArray, i, PyFloat_FromDouble( array[ i ] ) );
   }
   
   return pyArray;
}

