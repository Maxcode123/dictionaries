#include "ssdict.h"

// Allocate a dictionary (`__new__` method in Python).
static PyObject *SSDict__new__(PyTypeObject *type, PyObject *args,
                               PyObject *kwds) {
  SSDict *self;
  self = (SSDict *)type->tp_alloc(type, 0);
  return (PyObject *)self;
}

// Initialize a dictionary (`__init__` in Python).
static int SSDict__init__(PyObject *self) {
  SSDict *dict = (SSDict *)self;
  dict->head = NULL;
  dict->size = 0;

  return 0;
}

// Set a dictionary item (`__setitem__` method in Python).
static PyObject *SSDict__setitem__(PyObject *self, PyObject *args) {

  PyObject *key = NULL, *value = NULL;

  if (!PyArg_ParseTuple(args, "OO", &key, &value)) {
    return NULL;
  }

  if (_SSDict_set_item((SSDict *)self, key, value) < 0) {
    return NULL;
  }

  Py_RETURN_NONE;
}

// Get a dictionary item (`__getitem__` method in Python).
static PyObject *SSDict__getitem__(PyObject *self, PyObject *key) {

  Py_hash_t hash = PyObject_Hash(key);

  if (hash == -1) {
    PyObject_HashNotImplemented(key);
    return NULL;
  }

  SSDict *dict = (SSDict *)self;

  SSDictNode *node = dict->head;

  while (node != NULL) {
    if (node->key_hash == hash) {
      return node->value;
    }
    node = node->next;
  }

  PyErr_SetObject(PyExc_KeyError, key);
  return NULL;
}

// Get the length of the dictionary (`__len__` method in Python).
static PyObject *SSDict__len__(PyObject *self, int Py_UNUSED(_)) {
  SSDict *dict = (SSDict *)self;
  return PyLong_FromLong(dict->size);
}

// Deallocates the given dict. Frees all nodes.
static void _SSDict_dealloc(SSDict *self) {
  SSDictNode *node = self->head;
  SSDictNode *to_be_freed;

  while (node != NULL) {
    SSDictNode_dealloc(node);

    to_be_freed = node;
    node = node->next;
    free(to_be_freed);
  }
}

// Replaces the value of the given node. Increments the reference of the given
// value. Decrements the reference of the exisintg value.
static void _SSDict_replace_node_value(SSDictNode *node, PyObject *value) {
  // Use a temporary PyObject* instead of doing this:
  // Py_XDECREF(node->value);
  // Py_INCREF(value);
  // node->value = value;

  // `value` could be any object and the call to decrement its' reference could
  // trigger its' destructor which could do bad things.

  PyObject *tmp;

  tmp = node->value;

  Py_INCREF(value);
  node->value = value;

  Py_XDECREF(tmp);
}

static void _SSDict_add_new_node(SSDict *self, PyObject *key, PyObject *value) {
  SSDictNode *new = SSDictNode_new();
  if (new == NULL) {
    // node creation failed. do something
  }

  SSDictNode__init__(new, key, value);
  Py_INCREF(key);
  Py_INCREF(value);

  SSDictNode *node = self->head;
  self->head = new;
  new->next = node;
  self->size++;
}

// Assigns a new value to key or deletes the key.
static int _SSDict_assign_value(PyObject *_self, PyObject *key,
                                PyObject *value) {

  SSDict *self = (SSDict *)_self;

  if (value == NULL) {
    return _SSDict_delete_item(self, key);
  } else {
    return _SSDict_set_item(self, key, value);
  }
}

// Deletes an entry from the dictionary. Returns -1 upon failure.
static int _SSDict_delete_item(SSDict *self, PyObject *key) {
  Py_hash_t hash = PyObject_Hash(key);

  if (hash == -1) {
    PyObject_HashNotImplemented(key);
    return -1;
  }

  if (self->head->key_hash == hash) {
    return _SSDict_delete_head_node(self);
  }

  if (_SSDict_delete_body_node(self, hash) < 0) {
    PyErr_SetObject(PyExc_KeyError, key);
    return -1;
  }

  return 0;
}

// Deletes the head node of the dictionary and moves the head to the next node.
// Decrements the reference to the key and value of the head node.
static int _SSDict_delete_head_node(SSDict *self) {
  Py_DECREF(self->head->key);
  Py_DECREF(self->head->value);

  if (self->head->next == NULL) {
    free(self->head);
    self->head = NULL;
    self->size = 0;
  } else {
    SSDictNode *tmp = self->head;
    self->head = self->head->next;
    free(tmp);
    self->size--;
  }

  return 0;
}

// Deletes the node with the given hash from the the body of the list.
// Expects a valid hash (i.e. not equal to -1).
// Returns -1 if the node is not found.
static int _SSDict_delete_body_node(SSDict *self, Py_hash_t hash) {
  SSDictNode *before = self->head;
  SSDictNode *current = before->next;

  while (current != NULL) {
    if (current->key_hash == hash) {
      before->next = current->next;
      Py_DECREF(current->key);
      Py_DECREF(current->value);
      free(current);
      self->size--;

      return 0;
    }

    before = current;
    current = current->next;
  }

  return -1;
}

// Sets a dictionary entry. Returns -1 upon failure.
static int _SSDict_set_item(SSDict *self, PyObject *key, PyObject *value) {
  Py_hash_t hash = PyObject_Hash(key);

  if (hash == -1) {
    PyObject_HashNotImplemented(key);
    return -1;
  }

  SSDictNode *node = self->head;

  while (node != NULL) {
    if (node->key_hash == hash) {
      _SSDict_replace_node_value(node, value);

      return 0;
    }
    node = node->next;
  }

  _SSDict_add_new_node(self, key, value);

  return 0;
}

// Returns the number of items in the dictionary.
static int _SSDict_len(PyObject *_self) {
  SSDict *self = (SSDict *)_self;
  return self->size;
}

//-----------------------------------------------------------------------------
// TYPE DEFINITIONS
//-----------------------------------------------------------------------------

// SSDict methods array
static PyMethodDef SSDict_methods[] = {
    {"__setitem__", SSDict__setitem__, METH_VARARGS,
     "Set an item in the dictionary"},
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
    .sq_contains = 0,
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
