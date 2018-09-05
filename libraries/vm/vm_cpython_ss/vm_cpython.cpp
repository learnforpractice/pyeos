#include <map>

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
PyObject* PyInit_struct2(void);

PyThreadState* Py_NewInterpreterEx(void);
}

int cpython_setcode(uint64_t account, string& code);
int cpython_apply(unsigned long long receiver, unsigned long long account, unsigned long long action);
int init_function_whitelist();

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

extern "C" int PyImport_ImportFrozenModuleObjectEx(const struct _frozen *p);

extern "C" PyObject *PyImport_ImportFrozenModuleObjectExEx(const char *_name, const char *_code, int _size);

extern "C" PyObject *PyImport_LoadCodeObject(const char *_name, const char *_code, int _size);

PyObject* vm_load_module(string& name, string& bytecode) {
   return PyImport_ImportFrozenModuleObjectExEx(name.c_str(), bytecode.c_str(), bytecode.size());
}

PyObject* vm_load_codeobject(string& name, string& bytecodes) {
   return PyImport_LoadCodeObject(name.c_str(), bytecodes.c_str(), bytecodes.size());
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

#if 0
   PyThreadState_Swap(NULL);

//   substate = Py_NewInterpreterEx();
   substate = Py_NewInterpreter();
   {
      static unsigned char M___hello__[] = {
            0xe3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,
            0x00,0x40,0x00,0x00,0x00,0x73,0x0c,0x00,0x00,0x00,0x65,0x00,0x64,0x00,0x83,0x01,
            0x01,0x00,0x64,0x01,0x53,0x00,0x29,0x02,0x7a,0x12,0x68,0x65,0x6c,0x6c,0x6f,0x2c,
            0x77,0x6f,0x72,0x6c,0x64,0x64,0x64,0x64,0x64,0x64,0x64,0x64,0x4e,0x29,0x01,0xda,
            0x05,0x70,0x72,0x69,0x6e,0x74,0xa9,0x00,0x72,0x02,0x00,0x00,0x00,0x72,0x02,0x00,
            0x00,0x00,0xda,0x05,0x68,0x65,0x6c,0x6c,0x6f,0xda,0x08,0x3c,0x6d,0x6f,0x64,0x75,
            0x6c,0x65,0x3e,0x02,0x00,0x00,0x00,0x73,0x00,0x00,0x00,0x00,
      };

      #define SIZE (int)sizeof(M___hello__)

      PyImport_ImportFrozenModuleObjectExEx("hello", (char*)M___hello__, SIZE);
   }
#endif

   if (PyImport_ImportFrozenModule("__hello__") <= 0) {
       printf("can't import __hello__\n");
       exit(0);
   }

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

void memory_trace_stop();

struct sandbox {
   PyThreadState* state;
   std::map<string, PyObject*> modules;
};

static std::map<uint64_t, std::unique_ptr<sandbox>> s_sandbox_map;
static uint64_t s_current_account = 0;

extern "C" PyObject* vm_cpython_load_module(const char* module_name) {
   auto itr = s_sandbox_map.find(s_current_account);
   if (itr == s_sandbox_map.end()) {
      return NULL;
   }

   auto itr2 = itr->second->modules.find(module_name);
   if (itr2 == itr->second->modules.end()) {
      return NULL;
   }
   return itr2->second;
}

void prepare_env(uint64_t account) {
   auto itr = s_sandbox_map.find(account);
   if (itr == s_sandbox_map.end()) {
      PyThreadState_Swap(NULL);
      std::unique_ptr<sandbox> s = std::make_unique<sandbox>();
      s->state = Py_NewInterpreterEx();

      PyObject* module = PyInit_eoslib();
      s->modules["eoslib"] = module;

      module = PyInit_db();
      s->modules["db"] = module;

      module = PyInit_inspector();
      s->modules["inspector"] = module;

      module = PyInit_vm_cpython();
      s->modules["vm_cpython"] = module;

      module = PyInit_struct2();
      s->modules["_struct"] = module;


#if 0
      PyObject* module = PyImport_ImportModule("_struct");
      s->modules["_struct"] = module;

      module = PyImport_ImportModule("eoslib");
      s->modules["eoslib"] = module;

      module = PyImport_ImportModule("db");
      s->modules["db"] = module;

      module = PyImport_ImportModule("inspector");
      s->modules["inspector"] = module;

      module = PyImport_ImportModule("vm_cpython");
      s->modules["vm_cpython"] = module;
#endif
      s_sandbox_map[account] = std::move(s);
   } else {
      PyThreadState_Swap(itr->second->state);
   }
}

int vm_setcode(uint64_t account) {
   string code;
   get_code(account, code);
   s_current_account = account;

   memory_trace_stop();

   enable_injected_apis(0);
   enable_create_code_object(1);
   enable_filter_set_attr(0);
   enable_filter_get_attr(0);
   enable_inspect_obj_creation(0);

   prepare_env(account);

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
   s_current_account = receiver;

   memory_trace_stop();
   enable_injected_apis(0);
   enable_create_code_object(1);
   enable_filter_set_attr(0);
   enable_filter_get_attr(0);
   enable_inspect_obj_creation(0);

   prepare_env(receiver);

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


