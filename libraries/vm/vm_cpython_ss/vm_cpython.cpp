#include "vm_cpython.h"

#include <Python.h>

#include <eosiolib_native/vm_api.h>

#include <inspector/inspector.hpp>

static struct vm_api* s_api;

extern "C" {
PyObject* PyInit_eoslib();
PyObject* PyInit_db();
PyObject* PyInit_vm_cpython();
PyObject* PyInit_inspector();
PyObject* PyInit__struct(void);
int PyObject_GC_GetCount();
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

bool vm_cleanup() {
   if (PyObject_GC_GetCount() >=1000) {
      PyGC_Collect();
      return true;
   }
   return false;
}

int vm_run_script(const char* str) {
   return PyRun_SimpleString(str);
}

void vm_init(struct vm_api* api) {
   s_api = api;
   vm_register_api(api);

   setenv("PYTHONHOME", "../../libraries/vm/vm_cpython_ss", 1);
   setenv("PYTHONPATH", "../../libraries/vm/vm_cpython_ss/lib", 1);

   Py_NoSiteFlag = 1;
   PyImport_AppendInittab("_struct", PyInit__struct);
   PyImport_AppendInittab("eoslib", PyInit_eoslib);
   PyImport_AppendInittab("db", PyInit_db);
   PyImport_AppendInittab("inspector", PyInit_inspector);
   PyImport_AppendInittab("vm_cpython", PyInit_vm_cpython);

   Py_InitializeEx(0);
//   _Py_InitializeEx_Private(0, 0);

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
   api->vm_cleanup = vm_cleanup;
   api->vm_run_script = vm_run_script;
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

   enable_injected_apis(0);
   enable_create_code_object(1);
   enable_filter_set_attr(0);
   enable_filter_get_attr(0);
   enable_inspect_obj_creation(0);

   int ret = cpython_setcode(account, code);
   get_vm_api()->eosio_assert(ret, "setcode failed!");
   return 1;
}

int error_handler(string& error) {
   PyObject *_type;
   PyObject *_value;
   PyTracebackObject *_tb;
   if (!PyErr_Occurred()) {
      return 0;
   }
   PyErr_Fetch(&_type, &_value, (PyObject **)&_tb);
   if (_value == NULL || _tb == NULL) {
      return 0;
   }

   char buffer[256];
   Py_ssize_t size;
   const char *err = PyUnicode_AsUTF8AndSize(_value, &size);
   int n = snprintf(buffer, sizeof(buffer), "\n+++++traceback: %s\n", err);
   error = string(buffer, n);
   while (_tb) {
      PyCodeObject *code = _tb->tb_frame->f_code;
      const char* file_name = PyUnicode_AsUTF8(code->co_filename);
      const char* name = PyUnicode_AsUTF8(code->co_name);
      //_tb->tb_frame->f_lineno
      n = snprintf(buffer, sizeof(buffer), "%d %s %s\n", _tb->tb_lineno, file_name, name);
      error += string(buffer, n);
      _tb = _tb->tb_next;
   }
   PyErr_Clear();
   return 1;
}

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   enable_injected_apis(0);
   enable_create_code_object(1);
   enable_filter_set_attr(0);
   enable_filter_get_attr(0);
   enable_inspect_obj_creation(0);

   int ret = cpython_apply(receiver, account, act);
   if (ret == -1) {
      string error;
      error_handler(error);
      get_vm_api()->eosio_assert(0, error.c_str());
   }
   return 1;
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


