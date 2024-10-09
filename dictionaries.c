
#define PY_SSIZE_T_CLEAN
#include <python3.10/Python.h>

typedef unsigned int dict_size;
#define NO_ARGS int Py_UNUSED(_)

// Node of linked-list used for sequential-search dictionary implementation.
// This struct is not a Python object.
typedef struct _SSDictNode {
  PyObject *key;
  Py_hash_t key_hash; // Any hashable Python object can be a key
  PyObject *value;    // Any Python object can be a value
  struct _SSDictNode *next;
} SSDictNode;

// Dictionary object
typedef struct {
  PyObject_HEAD SSDictNode *head; // Head node of the linked-list.
  dict_size size;                 // Number of items in the dictionary
} SSDict;

// Deallocates the given node. Does not free the node, only decrements reference
// to key and value.
void SSDictNode_dealloc(SSDictNode *self) {
  Py_XDECREF(self->key);
  Py_XDECREF(self->value);
}

// Allocate a new node given the types of the key and value.
SSDictNode *SSDictNode_new() {
  SSDictNode *self = (SSDictNode *)PyMem_RawMalloc(sizeof(SSDictNode));
  if (self == NULL) {
    PyErr_SetString(PyExc_MemoryError,
                    "failed to allocate memory for SSDict node");
    return NULL;
  }

  self->key = NULL;
  self->key_hash = -1; // init value, a real hash could never be -1 because the
                       // hashing function PyObject_Hash returns -1 on failure
  self->value = NULL;

  return self;
}

// Initialize the values of the node. Increments the reference to the key and
// value.
void SSDictNode__init__(SSDictNode *self, PyObject *key, PyObject *value) {
  Py_hash_t hash = PyObject_Hash(key);

  if (hash == -1) {
    PyObject_HashNotImplemented(key);
    return;
  }

  self->key = Py_NewRef(key);

  self->key_hash = hash;

  self->value = Py_NewRef(value);
}

// Allocate a dictionary (`__new__` method in Python).
PyObject *SSDict__new__(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  SSDict *self;
  self = (SSDict *)type->tp_alloc(type, 0);
  return (PyObject *)self;
}

// Initialize a dictionary (`__init__` in Python).
int SSDict__init__(PyObject *self) {
  SSDict *dict = (SSDict *)self;
  dict->head = NULL;
  dict->size = 0;

  return 0;
}

// Deallocates the given dict. Frees all nodes.
void _SSDict_dealloc(SSDict *self) {
  SSDictNode *node = self->head;
  SSDictNode *to_be_freed;

  while (node != NULL) {
    SSDictNode_dealloc(node);

    to_be_freed = node;
    node = node->next;
    PyMem_RawFree(to_be_freed);
  }

  Py_TYPE(self)->tp_free((PyObject *)self);
}

// Replaces the value of the given node. Increments the reference of the given
// value. Decrements the reference of the exisintg value.
void _SSDict_replace_node_value(SSDictNode *node, PyObject *value) {
  // Use a temporary PyObject* instead of doing this:
  // Py_XDECREF(node->value);
  // Py_INCREF(value);
  // node->value = value;

  // `value` could be any object and the call to decrement its' reference could
  // trigger its' destructor which could do bad things.

  PyObject *tmp;

  tmp = node->value;

  node->value = Py_NewRef(value);

  Py_XDECREF(tmp);
}

void _SSDict_add_new_node(SSDict *self, PyObject *key, PyObject *value) {
  SSDictNode *new = SSDictNode_new();
  if (new == NULL) {
    // node creation failed. do something
  }

  SSDictNode__init__(new, key, value);

  SSDictNode *node = self->head;
  self->head = new;
  new->next = node;
  self->size++;
}

int _SSDict_delete_item(SSDict *self, PyObject *key);
int _SSDict_set_item(SSDict *self, PyObject *key, PyObject *value);

// Assigns a new value to key or deletes the key.
int _SSDict_assign_value(PyObject *_self, PyObject *key, PyObject *value) {

  SSDict *self = (SSDict *)_self;

  if (value == NULL) {
    return _SSDict_delete_item(self, key);
  } else {
    return _SSDict_set_item(self, key, value);
  }
}

// Deletes the head node of the dictionary and moves the head to the next node.
// Decrements the reference to the key and value of the head node.
int _SSDict_delete_head_node(SSDict *self) {
  Py_XDECREF(self->head->key);
  Py_XDECREF(self->head->value);

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
int _SSDict_delete_body_node(SSDict *self, Py_hash_t hash) {
  SSDictNode *before = self->head;
  SSDictNode *current = before->next;

  while (current != NULL) {
    if (current->key_hash == hash) {
      before->next = current->next;
      Py_XDECREF(current->key);
      Py_XDECREF(current->value);
      free(current);
      self->size--;

      return 0;
    }

    before = current;
    current = current->next;
  }

  return -1;
}

// Deletes an entry from the dictionary. Returns -1 upon failure.
int _SSDict_delete_item(SSDict *self, PyObject *key) {
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

// Sets a dictionary entry. Returns -1 upon failure.
int _SSDict_set_item(SSDict *self, PyObject *key, PyObject *value) {
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

// SSDict methods array
static PyMethodDef SSDict_methods[] = {{NULL}

};

static PyMappingMethods SSDict_mappingmethods = {
    .mp_length = 0,
    .mp_subscript = 0,
    .mp_ass_subscript = _SSDict_assign_value,
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
    .tp_hash = PyObject_HashNotImplemented};

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