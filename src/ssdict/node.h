#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>


// Node of linked-list used for sequential-search dictionary implementation.
// This struct is not a Python object.
typedef struct _SSDictNode {
  PyObject *key;
  Py_hash_t key_hash; // Any hashable Python object can be a key
  PyObject *value;    // Any Python object can be a value
  struct _SSDictNode *next;
} SSDictNode;


// Deallocates the given node. Does not free the node, only decrements reference
// to key and value.
void SSDictNode_dealloc(SSDictNode *self);

// Allocate a new node given the type of the value.
SSDictNode *SSDictNode_new();

// Initialize the values of the node. Increments the reference to the key and
// value.
void SSDictNode_init(SSDictNode *self, PyObject *key,
                               PyObject *value);