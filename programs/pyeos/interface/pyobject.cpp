/*
 * pyobject.cpp
 *
 *  Created on: Sep 8, 2017
 *      Author: newworld
 */
#include "pyobject.hpp"

PyArray::PyArray() { arr = array_create(); }
PyArray::~PyArray() { Py_XDECREF(arr); }

void PyArray::append(PyObject* obj) { array_append(arr, obj); }

void PyArray::append(std::string s) { array_append_string(arr, s); }

void PyArray::append(int n) { array_append_int(arr, n); }

void PyArray::append(unsigned int n) { array_append_int(arr, n); }

void PyArray::append(uint64_t n) { array_append_uint64(arr, n); }

void PyArray::append(double n) { array_append_double(arr, n); }

PyObject* PyArray::get() {
   Py_XINCREF(arr);
   return arr;
}

PyDict::PyDict() { pydict = dict_create(); }

PyDict::PyDict(PyObject* dictObj) { pydict = dictObj; }

PyDict::~PyDict() {
   Py_XDECREF(pydict);
}

void PyDict::add(PyObject* key, PyObject* value) {
   if (key == NULL || value == NULL) {
      return;
   }
   dict_add(pydict, key, value);
}

void PyDict::add(std::string& key, PyObject* value) {
   if (key.size() == 0 || value == NULL) {
      return;
   }
   PyObject* pykey = py_new_string(key);
   dict_add(pydict, pykey, value);
}

void PyDict::add(std::string& key, std::string& value) {
   PyObject* pykey = py_new_string(key);
   PyObject* pyvalue = py_new_string(value);
   dict_add(pydict, pykey, pyvalue);
}

void PyDict::add(std::string& key, long long n) {
   PyObject* pykey = py_new_string(key);
   PyObject* pyvalue = py_new_int64(n);
   dict_add(pydict, pykey, pyvalue);
}

PyObject* PyDict::get() {
   Py_XINCREF(pydict);
   return pydict;
}

