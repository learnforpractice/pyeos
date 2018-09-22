#include "vm_cpython.h"

#include <Python.h>


static struct vm_api* s_api;

extern "C" {
   PyObject* PyInit_vm_cpython();
   PyObject* PyInit_eoslib();
   PyObject* PyInit_db();
}

int cpython_setcode(uint64_t account, string& code);
int cpython_apply(uint64_t receiver, uint64_t account, uint64_t action);
int cpython_call(uint64_t account, uint64_t func);
void cpython_compile(string& name, string& code, string& result);

const char *vm_cpython_compile(const char *name, const char *code, int size, int *result_size) {
   static string _result;
   string _name(name);
   string _code(code, size);
   cpython_compile(_name, _code, _result);
   *result_size = _result.size();
   return _result.c_str();
}

void get_code(uint64_t account, string& code) {
   size_t size;
   const char* _code = get_vm_api()->get_code(account, &size);
   code = string(_code, size);
}

void vm_init(struct vm_api* api) {
   api->vm_cpython_compile = vm_cpython_compile;
   s_api = api;
   Py_InitializeEx(0);

   PyInit_db();
   PyInit_eoslib();
   PyInit_vm_cpython();
}

void vm_deinit() {
   printf("vm_python deinit\n");
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


