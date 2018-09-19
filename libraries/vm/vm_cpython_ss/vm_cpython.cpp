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
PyObject* PyInit_sys2(void);

PyObject* PyInit__tracemalloc(void);

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

static vector<char> print_buffer;

#define CONSOLE_GREEN "\033[32m"
#define CONSOLE_BLACK "\033[30m"

extern "C" void debug_print(const char* str, int len) {
   for (int i=0;i<len;i++) {
      if (str[i] == '\n') {
         time_t rawtime;
         struct tm * timeinfo;
         char buffer [32];

         time (&rawtime);
         timeinfo = localtime (&rawtime);
         strftime (buffer,80,"%T",timeinfo);

         string s(print_buffer.data(), print_buffer.size());

         PyFrameObject *f = PyThreadState_GET()->frame;
         if (f) {
            const char* filename = PyUnicode_AsUTF8(f->f_code->co_filename);
            const char* name = PyUnicode_AsUTF8(f->f_code->co_name);

            if(isatty(fileno(stdout))) {
               fprintf( stdout, "%s", CONSOLE_GREEN );
            }
            fprintf(stdout, "%-10s %4d %-20s %-20s %s", buffer, PyFrame_GetLineNumber(f), filename, name, s.c_str());
            fprintf(stdout, "%s\n", CONSOLE_BLACK);
         } else {
            fprintf(stdout, "%s\n", s.c_str());
         }
         print_buffer.clear();
         continue;
      }
      print_buffer.push_back(str[i]);
   }
}



int vm_run_script(const char* str) {
   return PyRun_SimpleString(str);
}

extern "C" int PyImport_ImportFrozenModuleObjectEx(const struct _frozen *p);

extern "C" PyObject *PyImport_ImportFrozenModuleObjectExEx(const char *_name, const char *_code, int _size);

extern "C" PyObject *PyImport_LoadCodeObject(const char *_name, const char *_code, int _size);

void prepare_env(uint64_t account);

PyObject* vm_load_module(string& name, string& bytecode) {
   return PyImport_ImportFrozenModuleObjectExEx(name.c_str(), bytecode.c_str(), bytecode.size());
}

PyObject* vm_load_codeobject(string& name, string& bytecodes) {
   return PyImport_LoadCodeObject(name.c_str(), bytecodes.c_str(), bytecodes.size());
}

void vm_init(struct vm_api* api) {
   s_api = api;
   vm_register_api(api);

   setenv("PYTHONHOME", "../../externals/python/dist", 1);
   setenv("PYTHONPATH", "../../externals/python/dist/lib", 1);

   Py_NoSiteFlag = 1;
   PyImport_AppendInittab("_struct", PyInit__struct);
   PyImport_AppendInittab("eoslib", PyInit_eoslib);
   PyImport_AppendInittab("db", PyInit_db);
   PyImport_AppendInittab("inspector", PyInit_inspector);
   PyImport_AppendInittab("vm_cpython", PyInit_vm_cpython);
   PyImport_AppendInittab("sys2", PyInit_sys2);

   Py_InitializeEx(0);

//   PyInit_eoslib();
//   PyInit_db();
//   PyInit_inspector();
//   PyInit_vm_cpython();
//   init_function_whitelist();

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

PyObject* load_module_from_db(uint64_t account, uint64_t code_name);

extern "C" PyObject* vm_cpython_load_module(const char* module_name) {
   auto itr = s_sandbox_map.find(s_current_account);
   if (itr == s_sandbox_map.end()) {
      return NULL;
   }

   auto itr2 = itr->second->modules.find(module_name);
   if (itr2 == itr->second->modules.end()) {
      uint64_t _module_name = get_vm_api()->string_to_uint64(module_name);
      return load_module_from_db(s_current_account, _module_name);//load module from code ext
   }
   return itr2->second;
}

void prepare_env(uint64_t account) {
   auto itr = s_sandbox_map.find(account);
   if (itr == s_sandbox_map.end()) {
      PyObject* module;
      PyObject* name;

      PyThreadState_Swap(NULL);
      std::unique_ptr<sandbox> s = std::make_unique<sandbox>();
      s->state = Py_NewInterpreterEx();

      module = PyInit__struct();
      s->modules["struct"] = module;
      name = PyUnicode_FromString("struct");
      _PyImport_SetModule(name, module);

      module = PyInit__tracemalloc();
      s->modules["_tracemalloc"] = module;
      name = PyUnicode_FromString("_tracemalloc");
      _PyImport_SetModule(name, module);

      module = PyInit_inspector();
      name = PyUnicode_FromString("inspector");
      _PyImport_SetModule(name, module);

      module = PyInit_vm_cpython();
      name = PyUnicode_FromString("vm_cpython");
      _PyImport_SetModule(name, module);

      module = PyInit_eoslib();
      name = PyUnicode_FromString("eoslib");
      _PyImport_SetModule(name, module);
      s->modules["eoslib"] = module;

      module = PyInit_db();
      name = PyUnicode_FromString("db");
      _PyImport_SetModule(name, module);
      s->modules["db"] = module;

      s_sandbox_map[account] = std::move(s);
   } else {
      PyThreadState_Swap(itr->second->state);
   }
}

int vm_setcode(uint64_t account) {
   string code;
   char name[32];
   int bytecode_size = 0;

   get_code(account, code);
   s_current_account = account;

   memory_trace_stop();

   enable_injected_apis(0);
   enable_create_code_object(1);
   enable_filter_set_attr(0);
   enable_filter_get_attr(0);
   enable_inspect_obj_creation(0);

   prepare_env(account);

   memset(name, 0, sizeof(name));
   get_vm_api()->uint64_to_string(account, name, sizeof(name));

   const char* bytecodes = get_vm_api()->vm_cpython_compile(name, code.c_str(), code.size(), &bytecode_size);
   eosio_assert(bytecode_size != 0, "compile python code failed!");

   get_vm_api()->set_code(account, VM_TYPE_PY, bytecodes, bytecode_size);

   code = string(bytecodes, bytecode_size);
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


