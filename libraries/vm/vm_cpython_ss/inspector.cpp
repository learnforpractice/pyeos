#include "inspector.hpp"
#include <eosiolib_native/vm_api.h>
#include <opcode.h>

using namespace std;

//vm_cpython.pyx
string get_c_string(PyObject* s);

struct opcode_map
{
   int index;
   const char* name;
};

extern struct opcode_map _opcode_map[];

inspector::inspector() {
   opcode_blacklist.resize(158, 0);//158
//   opcode_blacklist[IMPORT_NAME] = 1;
   opcode_blacklist[CALL_FUNCTION] = 1;
   opcode_blacklist[CALL_FUNCTION_KW] = 1;
   opcode_blacklist[CALL_FUNCTION_EX] = 1;

   import_name_whitelist["struct"] = 1;
   import_name_whitelist["_struct"] = 1;
   import_name_whitelist["db"] = 1;
   import_name_whitelist["eoslib"] = 1;
   for (int i=0;_opcode_map[i].index != 0;i++) {
      opcode_map[_opcode_map[i].index] = _opcode_map[i].name;
   }
}

inspector& inspector::get() {
   static inspector* inst = nullptr;
   if (!inst) {
      inst = new inspector();
   }
   return *inst;
}

int inspector::inspect_function(PyObject* func) {
   printf("inspect function %p\n", func);
   return function_whitelist.find(func) != function_whitelist.end();
}

int inspector::whitelist_function(PyObject* func) {
   function_whitelist[func] = func;
   return 1;
}

int inspector::whitelist_import_name(const char* name) {
   import_name_whitelist[name] = 1;
   return 1;
}

int inspector::inspect_import_name(const string& name) {
   return import_name_whitelist.find(name) != import_name_whitelist.end();
}

int inspector::whitelist_attr(const char* name) {

}

int inspector::inspect_attr(PyObject* name) {
   string _name = get_c_string(name);
   printf("++++++++++inspect_attr: %s \n", _name.c_str());
   if (_name == "__init__") {
      return true;
   }

   if (_name.size() >= 2 && _name[0] == '_' && _name[1] == '_') {
      printf("inspect_attr failed\n");
      return false;
   }

   return true;
}

int inspector::add_account_function(uint64_t account, PyObject* func) {
   if (account_functions.find(account) == account_functions.end()) {
      auto _new_map = std::make_unique<map<PyObject*, PyObject*>>();
      account_functions[account] = std::move(_new_map);
   }
   (*account_functions[account])[func] = func;
   return 1;
}

int inspector::whitelist_opcode(int opcode) {
   opcode_blacklist[opcode] = opcode;
   return 1;
}

int inspector::inspect_opcode(int opcode) {
   printf("inspect opcode %s \n", opcode_map[opcode]);
   return !opcode_blacklist[opcode];
}

void whitelist_function_(PyObject* func) {
   inspector::get().whitelist_function(func);
}

int inspect_function_(PyObject* func) {
   return inspector::get().inspect_function(func);
}

int whitelist_import_name_(const char* name) {
   return inspector::get().whitelist_import_name(name);
}

int inspect_import_name_(PyObject* name) {
   string _name = get_c_string(name);
   return inspector::get().inspect_import_name(_name);
}

int whitelist_attr_(const char* name) {
   return 0;
}

int inspect_attr_(PyObject* name) {
   return inspector::get().inspect_attr(name);
}

int whitelist_opcode_(int opcode) {
   return inspector::get().whitelist_opcode(opcode);
}


int inspect_opcode_(int opcode) {
   return inspector::get().inspect_opcode(opcode);
}

void enable_injected_apis_(int enabled) {
   get_injected_apis()->enabled = enabled;
}

void enable_opcode_inspector_(int enable) {
   if (enable) {
      get_injected_apis()->enable_opcode_inspect = 1;
   } else {
      get_injected_apis()->enable_opcode_inspect = 0;
   }
}

void enable_create_code_object_(int enable) {
   if (enable) {
      get_injected_apis()->enable_create_code_object = 1;
   } else {
      get_injected_apis()->enable_create_code_object = -1;
   }
}

int check_time_() {
   try {
      get_vm_api()->checktime();
   } catch (...) {
      return 0;
   }
   return 1;
}

void init_injected_apis() {
   struct python_injected_apis* apis = get_injected_apis();
   apis->enabled = 0;
   apis->status = 0;
   apis->enable_create_code_object = 0;
   apis->enable_opcode_inspect = 0;

   apis->whitelist_function = whitelist_function_;
   apis->inspect_function = inspect_function_;

   apis->whitelist_import_name = whitelist_import_name_;
   apis->inspect_import_name = inspect_import_name_;


   apis->whitelist_attr = whitelist_attr_;
   apis->inspect_attr = inspect_attr_;

   apis->inspect_opcode = inspect_opcode_;
   apis->check_time = check_time_;
}

void enable_injected_apis_() {
   get_injected_apis()->enabled = true;
}
