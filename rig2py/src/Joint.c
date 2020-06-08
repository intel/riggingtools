#include <stddef.h>
#include "Joint.h"
#include "structmember.h"

PyObject * jointType = NULL;

PyMemberDef pyJoint_members[] =
{
   { "name", T_OBJECT_EX, offsetof(pyJoint, name), 0, "name of this joint" },
   { "parentName", T_OBJECT_EX, offsetof(pyJoint, parentName), 0, "name of the parent joint" },
   { "offset", T_OBJECT_EX, offsetof(pyJoint, offset), 0, "offset vector of this joint relative to its parent" },
   { "quaternion", T_OBJECT_EX, offsetof(pyJoint, quaternion), 0, "rotatation of this joint relative to its parent and rest pose" },
   { "length", T_DOUBLE, offsetof(pyJoint, length), 0, "bone length" },
   { "index", T_INT, offsetof(pyJoint, index), 0, "joint index, starting at zero" },
   {NULL}  /* Sentinel */
};

PyObject * pyJoint_new( PyTypeObject * type,
   PyObject *args,
   PyObject *kwds )
{
   pyJoint *self;
   self = (pyJoint *)PyType_GenericAlloc(type, 0);
   if (self != NULL)
   {
      self->name = PyUnicode_FromString("");
      self->parentName = PyUnicode_FromString("");
      self->offset = Py_None;
      self->quaternion = Py_None;
      self->length = 1.0;
      self->index = 0;
   }

   (void)args;
   (void)kwds;
   return (PyObject *) self;
}

int pyJoint_init( pyJoint *self,
   PyObject *args,
   PyObject *kwds )
{
   PyObject * name = NULL,
      * parentName = NULL,
      * offset = NULL,
      * quaternion = NULL,
      * tmp;

   if ( !PyArg_ParseTuple( args,
      "OOOOdi",
      &name,
      &parentName,
      &offset,
      &quaternion,
      &self->length,
      &self->index ) )
   {
      return -1;
   }

   if ( name )
   {
      tmp = self->name;
      Py_INCREF(name);
      self->name = name;
      Py_XDECREF(tmp);
   }

   if ( parentName )
   {
      tmp = self->parentName;
      Py_INCREF(parentName);
      self->parentName = parentName;
      Py_XDECREF(tmp);
   }
   
   if ( offset )
   {
      tmp = self->offset;
      Py_INCREF(offset);
      self->offset = offset;
      Py_XDECREF(tmp);
   }
   
   if ( quaternion )
   {
      tmp = self->quaternion;
      Py_INCREF(quaternion);
      self->quaternion = quaternion;
      Py_XDECREF(tmp);
   }

   (void)kwds;
   return 0;
}

PyObject * pyJoint_repr( pyJoint * self )
{
   return PyUnicode_FromFormat( "%S_%d quat %S off %S",
      self->name,
      self->index,
      self->quaternion,
      self->offset );
}
void pyJoint_dealloc( pyJoint *self )
{
    Py_XDECREF(self->name);
    Py_XDECREF(self->parentName);
    Py_XDECREF(self->offset);
    Py_XDECREF(self->quaternion);
    PyObject_Del( (PyObject *)self );
}
PyObject * CreateJointType()
{
   static PyType_Slot slots[] =
   {
      { Py_tp_new,     (newfunc)pyJoint_new },
      { Py_tp_init,    (initproc)pyJoint_init },
      { Py_tp_dealloc, (destructor)pyJoint_dealloc },
      { Py_tp_repr,    (reprfunc)pyJoint_repr },
      { Py_tp_members, pyJoint_members },
      { 0, 0 }
   };
   PyType_Spec typeSpec =
   {
      .name = Py_STRINGIFY(MODULE_NAME) ".Joint",
      .basicsize = sizeof(pyJoint),
      .itemsize = 0,
      .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
      .slots = slots
   };

   return PyType_FromSpec( &typeSpec );
};


#ifdef __cplusplus
}
#endif
