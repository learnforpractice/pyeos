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

class inspector {
public:
   inspector();
   static inspector& get();

   int inspect_function(PyObject* func);
   int whitelist_function(PyObject* func);

   int whitelist_import_name(const char* name);
   int inspect_import_name(const string& name);

   int whitelist_attr(const char* name);
   int inspect_attr(PyObject* name);

   int whitelist_opcode(int opcode);
   int inspect_opcode(int opcode);

   int add_account_function(uint64_t account, PyObject* func);

   void enable_create_code_object_(int enable);


private:
   map<string, int> import_name_whitelist;
   map<int, const char*> opcode_map;

   map<PyObject*, PyObject*> function_whitelist;
   vector<int> opcode_blacklist;//158
   map<uint64_t, std::unique_ptr<map<PyObject*, PyObject*>>> account_functions;
};

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

#endif /* LIBRARIES_VM_VM_CPYTHON_SS_INSPECTOR_HPP_ */
