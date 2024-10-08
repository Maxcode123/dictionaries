#include "node.h"

// Deallocates the given node. Does not free the node, only decrements reference
// to key and value.
void SSDictNode_dealloc(SSDictNode *self) {
  Py_DECREF(self->key);
  Py_DECREF(self->value);
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

  self->key = key;
  self->key_hash = hash;
  self->value = value;
}