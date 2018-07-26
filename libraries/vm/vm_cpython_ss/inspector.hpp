/*
 * inspector.hpp
 *
 *  Created on: Jul 24, 2018
 *      Author: newworld
 */

#ifndef LIBRARIES_VM_VM_CPYTHON_SS_INSPECTOR_HPP_
#define LIBRARIES_VM_VM_CPYTHON_SS_INSPECTOR_HPP_


#include <stdio.h>
#include <stdint.h>
#include <Python.h>

#include <map>
#include <vector>
#include <memory>
#include <string>

#include <injector.h>

using namespace std;

class account_info {
public:
   map<PyObject*, PyObject*> account_functions;
   map<PyCodeObject*, int> code_objects;
};

class inspector {
public:
   inspector();
   static inspector& get();

   void set_current_account(uint64_t account);

   int inspect_function(PyObject* func);
   int whitelist_function(PyObject* func);

   int whitelist_import_name(const char* name);
   int inspect_import_name(const string& name);

   int whitelist_attr(const char* name);
   int inspect_setattr(PyObject* v, PyObject* name);

   int inspect_getattr(PyObject* v, PyObject* name);

   int whitelist_opcode(int opcode);
   int inspect_opcode(int opcode);

   int add_account_function(uint64_t account, PyObject* func);

   void add_code_object_to_current_account(PyCodeObject* co);
   int is_code_object_in_current_account(PyCodeObject* co);

   void enable_create_code_object_(int enable);

   void set_current_module(PyObject* mod);

private:
   PyObject* current_module;
   uint64_t current_account;
   map<string, int> import_name_whitelist;
   map<int, const char*> opcode_map;

   map<PyObject*, PyObject*> function_whitelist;
   vector<int> opcode_blacklist;//158
   map<uint64_t, std::unique_ptr<account_info>> accounts_info;
};

//account_functions

void init_injected_apis();

void enable_injected_apis_(int enabled);
void enable_opcode_inspector_(int enable);

void whitelist_function_(PyObject* func);
int inspect_function_(PyObject* func);

int whitelist_import_name_(const char* name);
int inspect_import_name_(const char*name);

void inspect_set_status_(int status);

void enable_create_code_object_(int enable);

void enable_injected_apis_();
void add_code_object_to_current_account_(PyCodeObject* co);
int is_code_object_in_current_account_(PyCodeObject* co);

void set_current_account_(uint64_t account);
void set_current_module_(PyObject* mod);

//object.c
extern "C" int attr_is_function(PyObject* v, PyObject* name);

#endif /* LIBRARIES_VM_VM_CPYTHON_SS_INSPECTOR_HPP_ */
