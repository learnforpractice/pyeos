/*
 * pyobject.cpp
 *
 *  Created on: Sep 8, 2017
 *      Author: newworld
 */
#include "pyobject.hpp"



PyArray::PyArray(){
    arr = array_create();
}

void PyArray::append(PyObject *obj){
	array_append(arr,obj);
}

void PyArray::append(std::string s){
	array_append_string(arr,s);
}

void PyArray::append(int n){
	array_append_int(arr,n);
}

void PyArray::append(unsigned int n){
	array_append_int(arr,n);
}

void PyArray::append(uint64_t n){
	array_append_uint64(arr,n);
}

void PyArray::append(double n){
	array_append_double(arr,n);
}

PyObject *PyArray::get(){
	return arr;
}


PyDict::PyDict(){
	pydict = dict_create();
}

void PyDict::add(PyObject* key,PyObject* value){
	if (key == NULL || value == NULL){
		return;
	}
	dict_add(pydict,key,value);
}

void PyDict::add(std::string& key,std::string& value){
	PyObject* pykey = py_new_string(key);
	PyObject* pyvalue = py_new_string(value);
	dict_add(pydict,pykey,pyvalue);
}

PyObject *PyDict::get(){
	return pydict;
}








