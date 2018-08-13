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
extern "C" PyThreadState *Py_NewInterpreterEx(void);

void get_code(uint64_t account, string& code) {
   size_t size;
   const char* _code = get_vm_api()->get_code(account, &size);
   code = string(_code, size);
}

void vm_init(struct vm_api* api) {
   s_api = api;
   vm_register_api(api);
   init_injected_apis();

   setenv("PYTHONHOME", "../../libraries/python-ss/dist", 1);
   setenv("PYTHONPATH", "../../libraries/python-ss/dist/lib/python3.6", 1);

   Py_NoSiteFlag = 1;
   PyImport_AppendInittab("_struct", PyInit__struct);
   PyImport_AppendInittab("eoslib", PyInit_eoslib);
   PyImport_AppendInittab("db", PyInit_db);
   PyImport_AppendInittab("inspector", PyInit_inspector);
   PyImport_AppendInittab("vm_cpython", PyInit_vm_cpython);

//   Py_InitializeEx(0);
   _Py_InitializeEx_Private(0, 0);

//   PyInit_eoslib();
//   PyInit_db();
//   PyInit_inspector();
//   PyInit_vm_cpython();
//   init_function_whitelist();


   PyThreadState *mainstate, *substate;
#ifdef WITH_THREAD
   PyGILState_STATE gilstate;
#endif

   mainstate = PyThreadState_Get();

#ifdef WITH_THREAD
   PyEval_InitThreads();
   PyEval_ReleaseThread(mainstate);

   gilstate = PyGILState_Ensure();
#endif

   PyThreadState_Swap(NULL);

//   substate = Py_NewInterpreterEx();
   substate = Py_NewInterpreter();

   PyImport_ImportModule("_struct");
   PyImport_ImportModule("eoslib");
   PyImport_ImportModule("db");
   PyImport_ImportModule("inspector");
   PyImport_ImportModule("vm_cpython");
//   enable_injected_apis_();

   init_function_whitelist();
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

int vm_call(uint64_t account, uint64_t func) {
   return 0;//cpython_call(account, func);
}

extern int cython_apply(PyObject* mod, unsigned long long receiver, unsigned long long account, unsigned long long action);

int vm_cpython_apply(PyObject* mod, unsigned long long receiver, unsigned long long account, unsigned long long action) {
   enable_injected_apis(1);
   enable_create_code_object(0);
   enable_filter_set_attr(1);
   enable_filter_get_attr(1);
   enable_inspect_obj_creation(1);

   int ret = 0;
   try {
      ret = cython_apply(mod, receiver, account, action);
   } catch (...) {
      enable_injected_apis(0);
      enable_create_code_object(1);
      enable_filter_set_attr(0);
      enable_filter_get_attr(0);
      enable_inspect_obj_creation(0);
      throw;
   }
   enable_injected_apis(0);
   enable_create_code_object(1);
   enable_filter_set_attr(0);
   enable_filter_get_attr(0);
   enable_inspect_obj_creation(0);
   return ret;
}


