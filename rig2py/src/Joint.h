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

// Python version of rig2c JOINT struct
typedef struct
{
   PyObject_HEAD
   PyObject * name;
   PyObject * parentName;
   PyObject * offset;
   PyObject * quaternion;
   double length;
   int index;
}pyJoint;

PyObject * pyJoint_new( PyTypeObject *type,
   PyObject *args,
   PyObject *kwds );
int pyJoint_init( pyJoint * self,
   PyObject *args,
   PyObject *kwds );
void pyJoint_dealloc( pyJoint * self );
PyObject * pyJoint_repr( pyJoint * self );
PyObject * CreateJointType();

extern PyObject * jointType;

#ifdef __cplusplus
}
#endif
