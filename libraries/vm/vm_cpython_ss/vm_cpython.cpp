#include "vm_cpython.h"

#include <Python.h>

#include <eosiolib_native/vm_api.h>

static struct vm_api* s_api;

extern "C" {
PyObject* PyInit_eoslib();
PyObject* PyInit_db();
PyObject* PyInit_vm_cpython();
}

int init_cpython();
int cpython_setcode(uint64_t account, string& code);
int cpython_apply(unsigned long long receiver, unsigned long long account, unsigned long long action);

void get_code(uint64_t account, string& code) {
   size_t size;
   const char* _code = get_vm_api()->get_code(account, &size);
   code = string(_code, size);
}

int init_cpython_() {
   Py_InitializeEx(0);
   PyInit_vm_cpython();
   return 1;
}

void vm_init(struct vm_api* api) {
   s_api = api;
   vm_register_api(api);
   Py_InitializeEx(0);
   PyRun_SimpleString("import sys;sys.path.append('/Users/newworld/dev/pyeos/libraries/python-ss/Lib')");
   PyInit_eoslib();
   PyInit_db();
   PyInit_vm_cpython();
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



