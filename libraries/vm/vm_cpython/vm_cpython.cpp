#include "vm_cpython.h"

#include <Python.h>


static struct vm_api* s_api;

extern "C" {
   PyObject* PyInit_vm_cpython();
   PyObject* PyInit_eoslib();
   PyObject* PyInit_db();
}

int init_cpython();
int cpython_setcode(uint64_t account, string& code);
int cpython_apply(unsigned long long receiver, unsigned long long account, unsigned long long action);
int cpython_call(uint64_t account, uint64_t func);

void get_code(uint64_t account, string& code) {
   size_t size;
   const char* _code = get_vm_api()->get_code(account, &size);
   code = string(_code, size);
}

int init_cpython_() {
   PyInit_vm_cpython();
   return 1;
}

void vm_init(struct vm_api* api) {
   s_api = api;
   vm_register_api(api);
   PyInit_db();
   PyInit_eoslib();
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

   #ifdef WITH_THREAD
   PyGILState_STATE save = PyGILState_Ensure();
   #endif
   int ret;
   try {
      ret = cpython_setcode(account, code);
   } catch (...) {
      #ifdef WITH_THREAD
      PyGILState_Release(save);
      #endif
      throw;
   }
   #ifdef WITH_THREAD
   PyGILState_Release(save);
   #endif
   return ret;

}

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   #ifdef WITH_THREAD
   PyGILState_STATE save = PyGILState_Ensure();
   #endif
   int ret;
   try {
      ret = cpython_apply(receiver, account, act);
   } catch (...) {
      #ifdef WITH_THREAD
      PyGILState_Release(save);
      #endif
      throw;
   }
   #ifdef WITH_THREAD
   PyGILState_Release(save);
   #endif
   return ret;
}

int vm_call(uint64_t account, uint64_t func) {
   return cpython_call(account, func);
}


