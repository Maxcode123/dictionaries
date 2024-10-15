#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "node.h"

typedef unsigned int dict_size;
#define NO_ARGS int Py_UNUSED(_)

//-----------------------------------------------------------------------------
// PYTHON TYPES
//-----------------------------------------------------------------------------

// Dictionary object
typedef struct {
  PyObject_HEAD SSDictNode *head; // Head node of the linked-list.
  dict_size size;                 // Number of items in the dictionary
} SSDict;

//-----------------------------------------------------------------------------
// METHODS
//-----------------------------------------------------------------------------

// Allocate a dictionary (`__new__` method in Python).
PyObject *SSDict__new__(PyTypeObject *type, PyObject *args, PyObject *kwds);

// Initialize a dictionary (`__init__` in Python).
int SSDict__init__(PyObject *self);

// Get a dictionary item (`__getitem__ method in Python).
PyObject *SSDict__getitem__(PyObject *self, PyObject *key);

// Deallocates the given dict. Frees all nodes.
void SSDict_dealloc(SSDict *self);

// Replaces the value of the given node. Increments the reference of the given
// value. Decrements the reference of the existing value.
void _SSDict_replace_node_value(SSDictNode *node, PyObject *value);

// Adds a new entry-node to the dictionary. Increments the reference to the key
// and value.
void _SSDict_add_new_node(SSDict *self, PyObject *key, PyObject *value);

// Assigns a new value to key or deletes the key if the value is NULL.
int _SSDict_assign_value(PyObject *_self, PyObject *key, PyObject *value);

// Deletes an entry from the dictionary. Returns -1 upon failure.
int _SSDict_delete_item(SSDict *self, PyObject *key);

// Deletes the head node of the dictionary and moves the head to the next node.
// Decrements the reference to the key and value of the head node.
int _SSDict_delete_head_node(SSDict *self);

// Deletes the node with the given hash from the the body of the list.
// Expects a valid hash (i.e. not equal to -1).
// Returns -1 if the node is not found.
int _SSDict_delete_body_node(SSDict *self, Py_hash_t hash);

// Sets a dictionary entry. Returns -1 upon failure.
int _SSDict_set_item(SSDict *self, PyObject *key, PyObject *value);

// Returns the number of items in the dictionary.
int _SSDict_len(PyObject *_self);

// Returns 1 if the dictionary contains the given key, 0 otherwise. Returns -1
// upon failure.
int SSDict__contains__(PyObject *_self, PyObject *key);
