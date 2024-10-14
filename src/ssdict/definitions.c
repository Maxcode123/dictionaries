#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "ssdict.h"

// SSDict methods array
static PyMethodDef SSDict_methods[] = {
    {"__getitem__", SSDict__getitem__, METH_O | METH_COEXIST,
     "Get an item from the dictionary"},
    {NULL}

};

static PyMappingMethods SSDict_mappingmethods = {
    .mp_length = (lenfunc)_SSDict_len,
    .mp_subscript = SSDict__getitem__,
    .mp_ass_subscript = _SSDict_assign_value,
};

/* Hack to implement "key in dict" */
static PySequenceMethods SSDict_sequencemethods = {
    .sq_length = 0,
    .sq_concat = 0,
    .sq_repeat = 0,
    .sq_item = 0,
    .sq_ass_item = 0,
    .sq_contains = SSDict__contains__,
    .sq_inplace_concat = 0,
    .sq_inplace_repeat = 0,
};

// Dictionary type object (the `type(SSDict)` object in Python)
static PyTypeObject SSDictType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0).tp_name = "dictionaries.SSDict",
    .tp_doc =
        PyDoc_STR("Dictionary that uses a sequential-search linked-list."),
    .tp_basicsize = sizeof(SSDict),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = SSDict__new__,
    .tp_init = (initproc)SSDict__init__,
    .tp_dealloc = (destructor)_SSDict_dealloc,
    .tp_methods = SSDict_methods,
    .tp_as_mapping = &SSDict_mappingmethods,
    .tp_as_sequence = &SSDict_sequencemethods,
    .tp_hash = PyObject_HashNotImplemented};

//-----------------------------------------------------------------------------
// MODULE DEFINITIONS
//-----------------------------------------------------------------------------

// Module struct
static PyModuleDef dictionariesmodule = {.m_base = PyModuleDef_HEAD_INIT,
                                         .m_name = "dictionaries",
                                         .m_doc = "Dictionary types.",
                                         .m_size = -1};

// Module initialization function
PyMODINIT_FUNC PyInit_dictionaries(void) {
  PyObject *module;
  if (PyType_Ready(&SSDictType) < 0)
    return NULL;

  module = PyModule_Create(&dictionariesmodule);
  if (module == NULL)
    return NULL;

  Py_INCREF(&SSDictType);
  if (PyModule_AddObject(module, "SSDict", (PyObject *)&SSDictType) < 0) {
    Py_DECREF(&SSDictType);
    Py_DECREF(module);
    return NULL;
  }

  return module;
}