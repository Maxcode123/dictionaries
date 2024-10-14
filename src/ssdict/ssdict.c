#include "ssdict.h"

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

// Get a dictionary item (`__getitem__` method in Python).
PyObject *SSDict__getitem__(PyObject *self, PyObject *key) {

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

// Assigns a new value to key or deletes the key.
int _SSDict_assign_value(PyObject *_self, PyObject *key, PyObject *value) {

  SSDict *self = (SSDict *)_self;

  if (value == NULL) {
    return _SSDict_delete_item(self, key);
  } else {
    return _SSDict_set_item(self, key, value);
  }
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

// Returns the number of items in the dictionary.
int _SSDict_len(PyObject *_self) {
  SSDict *self = (SSDict *)_self;
  return self->size;
}

// Returns 1 if the dictionary contains the given key, 0 otherwise. Returns -1
// upon failure.
int SSDict__contains__(PyObject *_self, PyObject *key) {
  Py_hash_t hash = PyObject_Hash(key);

  if (hash == -1) {
    PyObject_HashNotImplemented(key);
    return -1;
  }

  SSDict *self = (SSDict *)_self;

  SSDictNode *node = self->head;

  while (node != NULL) {
    if (node->key_hash == hash) {
      return 1;
    }

    node = node->next;
  }

  return 0;
}
