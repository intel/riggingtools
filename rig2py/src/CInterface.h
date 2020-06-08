#define PY_SSIZE_T_CLEAN

// Annoying work-around to avoid Windows incorrectly linking to a debug
// python library which isn't available
#if defined(_MSC_VER) && defined(_DEBUG)
   #undef _DEBUG
   #include <Python.h>
   #define _DEBUG
#else
   #include <Python.h>
#endif

#ifdef __cplusplus
extern "C"{
#endif

extern PyObject* g_boundsCallback;
extern PyObject* g_frameCallback;
extern PyObject* g_errorCallback;

PyObject * rig2python_errorCallback( PyObject *, PyObject * );
PyObject * rig2python_boundsCallback( PyObject *, PyObject * );
PyObject * rig2python_frameCallback( PyObject *, PyObject * );
PyObject * rig2python_getInfo( PyObject *, PyObject * );
PyObject * rig2python_getRigInfo( PyObject *, PyObject * );
PyObject * rig2python_getRestPoseHumanoid( PyObject * py, PyObject * args );
PyObject * rig2python_read( PyObject *, PyObject * );
void Initialize(void);
PyObject * BuildPythonArray( const double * array, size_t arraySize );

#ifdef __cplusplus
}
#endif
