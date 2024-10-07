#define PY_SSIZE_T_CLEAN
#include <python3.10/Python.h>

typedef unsigned int dict_size;
#define NO_ARGS int Py_UNUSED(_)

//-----------------------------------------------------------------------------
// INTERNAL TYPES
//-----------------------------------------------------------------------------

// Node of linked-list used for sequential-search dictionary implementation.
// This struct is not a Python object.
typedef struct _SSDictNode {
  PyObject *key;
  Py_hash_t key_hash; // Any hashable Python object can be a key
  PyObject *value;    // Any Python object can be a value
  struct _SSDictNode *next;
} SSDictNode;

//-----------------------------------------------------------------------------
// PYTHON TYPES
//-----------------------------------------------------------------------------

// Dictionary object
typedef struct {
  PyObject_HEAD SSDictNode *head; // Head node of the linked-list.
  dict_size size;                 // Number of items in the dictionary
} SSDict;

// Dictionary keys collection object
typedef struct {} SSDictKeys;

// Dictionary values collection object
typedef struct {} SSDictValues;

//-----------------------------------------------------------------------------
// METHODS
//-----------------------------------------------------------------------------

// Deallocate the given node.
static void SSDictNode_dealloc(SSDictNode *self);

// Allocate a new node given the type of the value.
static SSDictNode *SSDictNode_new();

// Initialize the values of the node. Increments the reference to the key and
// value.
static void SSDictNode__init__(SSDictNode *self, PyObject *key,
                               PyObject *value);

// Deallocate the given dict.
static void SSDict_dealloc(SSDict *self);

//-----------------------------------------------------------------------------
// PUBLIC API METHODS
//-----------------------------------------------------------------------------

// Allocate a dictionary (`__new__` method in Python).
static PyObject *SSDict__new__(PyTypeObject *type, PyObject *args,
                               PyObject *kwds);

// Initialize a dictionary (`__init__` in Python).
static PyObject *SSDict__init__(PyObject *self);

// Set a dictionary item (`__setitem__` method in Python).
static PyObject *SSDict__setitem__(PyObject *self, PyObject *args);

// Get a dictionary item (`__getitem__ method in Python).
static PyObject *SSDict__getitem__(PyObject *self, PyObject *key);

// Get the length of the dictionary (`__len__` method in Python).
static PyObject *SSDict__len__(PyObject *self, NO_ARGS);

//-----------------------------------------------------------------------------
// PRIVATE METHODS
//-----------------------------------------------------------------------------

// Replaces the value of the given node. Increments the reference of the given
// value. Decrements the reference of the existing value.
static void _SSDict_replace_node_value(SSDictNode *node, PyObject *value);

// Adds a new entry-node to the dictionary. Increments the reference to the key
// and value.
static void _SSDict_add_new_node(SSDict *self, PyObject *key, PyObject *value);

// Assigns a new value to key or deletes the key if the value is NULL.
static int _SSDict_assign_value(PyObject *_self, PyObject *key, PyObject *value);

// Deletes an entry from the dictionary. Returns -1 upon failure.
static int _SSDict_delete_item(SSDict *self, PyObject *key);

// Deletes the head node of the dictionary and moves the head to the next node.
// Decrements the reference to the key and value of the head node.
static int _SSDict_delete_head_node(SSDict *self);

// Deletes the node with the given hash from the the body of the list.
// Expects a valid hash (i.e. not equal to -1).
// Returns -1 if the node is not found.
static int _SSDict_delete_body_node(SSDict *self, Py_hash_t hash);

// Sets a dictionary entry. Returns -1 upon failure.
static int _SSDict_set_item(SSDict *self, PyObject *key, PyObject *value);
