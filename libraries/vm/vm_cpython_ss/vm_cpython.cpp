#include "vm_cpython.h"

#include <Python.h>

#include <eosiolib_native/vm_api.h>

#include "inspector.hpp"

static struct vm_api* s_api;

extern "C" {
PyObject* PyInit_eoslib();
PyObject* PyInit_db();
PyObject* PyInit_vm_cpython();
PyObject* PyInit_inspector();
PyObject* PyInit__struct(void);
}

int cpython_setcode(uint64_t account, string& code);
int cpython_apply(unsigned long long receiver, unsigned long long account, unsigned long long action);
int init_function_whitelist();

void get_code(uint64_t account, string& code) {
   size_t size;
   const char* _code = get_vm_api()->get_code(account, &size);
   code = string(_code, size);
}

void vm_init(struct vm_api* api) {
   s_api = api;
   vm_register_api(api);
   init_injected_apis();

   Py_NoSiteFlag = 1;
   PyImport_AppendInittab("_struct", PyInit__struct);

   Py_InitializeEx(0);

   PyImport_ImportModule("_struct");

   PyRun_SimpleString("import struct");
   PyInit_eoslib();
   PyInit_db();
   PyInit_inspector();
   PyInit_vm_cpython();
   init_function_whitelist();

   enable_injected_apis_();
}

void vm_deinit() {
   printf("vm_native finalize\n");
}

struct vm_api* get_vm_api() {
   return s_api;
}

int vm_setcode(uint64_t account) {
   string code;
   get_code(account, code);
   return cpython_setcode(account, code);
}

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   return cpython_apply(receiver, account, act);
}

int vm_preload(uint64_t account) {
   return 0;
}

int vm_unload(uint64_t account) {
   return 0;
}



