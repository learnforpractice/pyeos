#include "inspector.hpp"
#include <eosiolib_native/vm_api.h>
#include <opcode.h>

using namespace std;

//vm_cpython.pyx
string get_c_string(PyObject* s);
int py_inspect_getattr(PyObject* v, PyObject* name);
int py_inspect_setattr(PyObject* v, PyObject* name);
int py_inspect_function(PyObject* func);

extern "C" {
   extern PyTypeObject PyStructType;
   extern PyTypeObject PyFileIO_Type;
   extern PyTypeObject PyBufferedReader_Type;
   extern PyTypeObject PyTextIOWrapper_Type;
}

struct opcode_map
{
   int index;
   const char* name;
};

extern struct opcode_map _opcode_map[];

inspector::inspector() {
   current_module = nullptr;
   current_account = 0;
   opcode_blacklist.resize(158, 0);//158
//   opcode_blacklist[IMPORT_NAME] = 1;
//   opcode_blacklist[CALL_FUNCTION] = 1;
//   opcode_blacklist[CALL_FUNCTION_KW] = 1;
//   opcode_blacklist[CALL_FUNCTION_EX] = 1;

   import_name_whitelist["struct"] = 1;
   import_name_whitelist["_struct"] = 1;
   import_name_whitelist["db"] = 1;
   import_name_whitelist["eoslib"] = 1;
   for (int i=0;_opcode_map[i].index != 0;i++) {
      opcode_map[_opcode_map[i].index] = _opcode_map[i].name;
   }

   type_whitelist_map[&PyBaseObject_Type] = 1;
   type_whitelist_map[&PyType_Type] = 1;
   type_whitelist_map[&_PyWeakref_RefType] = 1;
   type_whitelist_map[&_PyWeakref_CallableProxyType] = 1;
   type_whitelist_map[&_PyWeakref_ProxyType] = 1;
   type_whitelist_map[&PyLong_Type] = 1;
   type_whitelist_map[&PyBool_Type] = 1;
   type_whitelist_map[&PyByteArray_Type] = 1;
   type_whitelist_map[&PyBytes_Type] = 1;
   type_whitelist_map[&PyList_Type] = 1;
   type_whitelist_map[&_PyNone_Type] = 1;
   type_whitelist_map[&_PyNotImplemented_Type] = 1;
   type_whitelist_map[&PyTraceBack_Type] = 1;
   type_whitelist_map[&PySuper_Type] = 1;
   type_whitelist_map[&PyRange_Type] = 1;
   type_whitelist_map[&PyDict_Type] = 1;
   type_whitelist_map[&PyDictKeys_Type] = 1;
   type_whitelist_map[&PyDictValues_Type] = 1;
   type_whitelist_map[&PyDictItems_Type] = 1;
   type_whitelist_map[&PyODict_Type] = 1;
   type_whitelist_map[&PyODictKeys_Type] = 1;
   type_whitelist_map[&PyODictItems_Type] = 1;
   type_whitelist_map[&PyODictValues_Type] = 1;
   type_whitelist_map[&PyODictIter_Type] = 1;
   type_whitelist_map[&PySet_Type] = 1;
   type_whitelist_map[&PyUnicode_Type] = 1;
   type_whitelist_map[&PySlice_Type] = 1;
   type_whitelist_map[&PyStaticMethod_Type] = 1;
   type_whitelist_map[&PyComplex_Type] = 1;
   type_whitelist_map[&PyFloat_Type] = 1;
   type_whitelist_map[&PyFrozenSet_Type] = 1;
   type_whitelist_map[&PyProperty_Type] = 1;
   type_whitelist_map[&_PyManagedBuffer_Type] = 1;
   type_whitelist_map[&PyMemoryView_Type] = 1;
   type_whitelist_map[&PyTuple_Type] = 1;
   type_whitelist_map[&PyEnum_Type] = 1;
   type_whitelist_map[&PyReversed_Type] = 1;
   type_whitelist_map[&PyStdPrinter_Type] = 1;
//   type_whitelist_map[&PyCode_Type] = 1;
   type_whitelist_map[&PyFrame_Type] = 1;
   type_whitelist_map[&PyCFunction_Type] = 1;
   type_whitelist_map[&PyMethod_Type] = 1;
   type_whitelist_map[&PyFunction_Type] = 1;
   type_whitelist_map[&PyDictProxy_Type] = 1;
   type_whitelist_map[&PyGen_Type] = 1;
   type_whitelist_map[&PyGetSetDescr_Type] = 1;
   type_whitelist_map[&PyWrapperDescr_Type] = 1;
   type_whitelist_map[&_PyMethodWrapper_Type] = 1;
   type_whitelist_map[&PyEllipsis_Type] = 1;
   type_whitelist_map[&PyMemberDescr_Type] = 1;
   type_whitelist_map[&_PyNamespace_Type] = 1;
   type_whitelist_map[&PyCapsule_Type] = 1;
   type_whitelist_map[&PyLongRangeIter_Type] = 1;
   type_whitelist_map[&PyCell_Type] = 1;
   type_whitelist_map[&PyInstanceMethod_Type] = 1;
   type_whitelist_map[&PyClassMethodDescr_Type] = 1;
   type_whitelist_map[&PyMethodDescr_Type] = 1;
   type_whitelist_map[&PyCallIter_Type] = 1;
   type_whitelist_map[&PySeqIter_Type] = 1;
   type_whitelist_map[&PyCoro_Type] = 1;
   type_whitelist_map[&_PyCoroWrapper_Type] = 1;

   type_whitelist_map[(PyTypeObject*)PyExc_Exception] = 1;
   type_whitelist_map[(PyTypeObject*)PyExc_TypeError] = 1;
   type_whitelist_map[(PyTypeObject*)PyExc_MemoryError] = 1;
   type_whitelist_map[(PyTypeObject*)PyExc_AttributeError] = 1;
   type_whitelist_map[(PyTypeObject*)PyExc_RecursionError] = 1;
   type_whitelist_map[(PyTypeObject*)PyExc_OSError] = 1;
   type_whitelist_map[(PyTypeObject*)PyExc_AssertionError] = 1;
   type_whitelist_map[(PyTypeObject*)PyExc_ImportError] = 1;

   type_whitelist_map[(PyTypeObject*)PyExc_NameError] = 1;
   type_whitelist_map[(PyTypeObject*)PyExc_IndexError] = 1;

   type_whitelist_map[&PyFileIO_Type] = 1; //FIXME: dangerous type
   type_whitelist_map[&PyBufferedReader_Type] = 1;
   type_whitelist_map[&PyTextIOWrapper_Type] = 1;

   type_whitelist_map[&PyStructType] = 1;
}

inspector& inspector::get() {
   static inspector* inst = nullptr;
   if (!inst) {
      inst = new inspector();
   }
   return *inst;
}

int inspector::inspect_obj_creation(PyTypeObject* type) {
   if (type_whitelist_map.find(type) != type_whitelist_map.end()) {
      return 1;
   }
   if (is_class_in_current_account((PyObject*)type)) {
      return 1;
   }
   return 0;
}

int inspector::add_type_to_whitelist(PyTypeObject* type) {
   type_whitelist_map[type] = 1;
   return 1;
}

void inspector::set_current_account(uint64_t account) {
   current_account = account;

   auto it = accounts_info_map.find(account);
   if (it == accounts_info_map.end()) {
      auto _new_map = std::make_shared<account_info>();
      _new_map->account = account;
      accounts_info_map[account] = _new_map;
      it = accounts_info_map.find(account);
   }
}

int inspector::inspect_function(PyObject* func) {
//   printf("++++++inspector::inspect_function: %p\n", func);
   return py_inspect_function(func);
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
   return 1;
}

bool compile_string(Py_UNICODE * wstr, Py_ssize_t size, string& str) {
   if (size != str.size()) {
      return false;
   }
   for(int i=0;i<size;i++) {
      if (wstr[i] != str[i]) {
         return false;
      }
   }
   return true;
}

int inspector::inspect_setattr(PyObject* v, PyObject* name) {
   Py_ssize_t size;
   Py_UNICODE * wstr = PyUnicode_AsUnicodeAndSize(name, &size);
   if (!wstr) {
      return 0;
   }

   if (size >= 2 && wstr[0] == '_' && wstr[1] == '_') {
      printf("inspect_setattr failed\n");
      return 0;
   }

   if (current_module == v) {
      return 1;
   }

   PyObject* cls = (PyObject*)Py_TYPE(v);
   if (is_class_in_current_account(cls)) {
      return 1;
   }
   return 0;

   string s("__init__");
   if (compile_string(wstr, size, s)) {
      return 1;
   }


   if (size >= 2 && wstr[0] == '_' && wstr[1] == '_') {
      printf("inspect_setattr failed\n");
      return 0;
   } else {
      return 1;
   }

   return 1;
}

int inspector::inspect_getattr(PyObject* v, PyObject* name) {

   Py_ssize_t size;
   Py_UNICODE * wstr = PyUnicode_AsUnicodeAndSize(name, &size);
   if (!wstr) {
      return 0;
   }

   string s("__init__");
   if (compile_string(wstr, size, s)) {
      return 1;
   }

   if (size >= 2 && wstr[0] == '_' && wstr[1] == '_') {
      printf("inspect_getattr failed\n");
      return 0;
   }

   if (current_module == v) {
      return 1;
   }

   PyObject* cls = (PyObject*)Py_TYPE(v);
   if (is_class_in_current_account(cls)) {
      return 1;
   }
   return py_inspect_getattr(v, name);
}

int inspector::inspect_build_class(PyObject* cls) {
   PyFrameObject *f = PyThreadState_GET()->frame;
   if (f != NULL) {
      auto it = accounts_info_map.find(current_account);
      if (it == accounts_info_map.end()) {
         printf("+++inspect_build_class account not found!");
         return 0;
      }

      auto& info = it->second;
      if (info->code_objects.find(f->f_code) != info->code_objects.end()) {
//         printf("-----add cls to info %p\n", cls);
         info->class_objects[cls] = 1;
         return 1;
      } else {
         printf("-----add cls to info %p, code object not in current account\n", cls);
      }
   }
   return 0;
}

int inspector::is_class_in_current_account(PyObject* cls) {
   auto it = accounts_info_map.find(current_account);
   if (it != accounts_info_map.end()) {
      auto& info = it->second;
      return info->class_objects.find(cls) != info->class_objects.end();
   }
   return 0;
}

int inspector::add_account_function(uint64_t account, PyObject* func) {
   auto it = accounts_info_map.find(account);
   if (it == accounts_info_map.end()) {
      auto _new_map = std::make_shared<account_info>();
      _new_map->account = current_account;
      accounts_info_map[account] = _new_map;
      it = accounts_info_map.find(account);
   }
   it->second->account_functions[func] = func;
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

void inspector::add_code_object_to_current_account(PyCodeObject* co) {
   auto it = accounts_info_map.find(current_account);
   it->second->code_objects[co] = 1;
}

int inspector::is_code_object_in_current_account(PyCodeObject* co) {
   auto it = accounts_info_map.find(current_account);
   if (it == accounts_info_map.end()) {
      return 0;
   }

   auto& code_objs = it->second->code_objects;
   if (code_objs.find(co) == code_objs.end()) {
      return 0;
   }
   return 1;
}

void inspector::set_current_module(PyObject* mod) {
   current_module = mod;
}

void inspector::memory_trace_start() {

}

void inspector::memory_trace_stop() {

}

void inspector::memory_trace_alloc(void* ptr, size_t new_size) {
   if (!current_account) {
      return;
   }

   auto _account_info = accounts_info_map.find(current_account);
   if (_account_info == accounts_info_map.end()) {
      return;
   }
//   printf("++++++++++memory_trace_alloc %lld %p %d\n", current_account, ptr, new_size);
   memory_info_map[ptr] = std::make_unique<account_memory_info>();

   auto& _memory_info = memory_info_map[ptr];
   _account_info->second->total_used_memory += new_size;
   _memory_info->size = new_size;
   _memory_info->info = _account_info->second;

//   memory_info_map[ptr] = std::move(_memory_info);
}

void inspector::memory_trace_realloc(void* old_ptr, void* new_ptr, size_t new_size) {
   if (!current_account) {
      return;
   }
   auto _account_info = accounts_info_map.find(current_account);
   if (_account_info == accounts_info_map.end()) {
      return;
   }

   auto it = memory_info_map.find(old_ptr);
   if (it == memory_info_map.end()) {
      return;
   }
   it->second->info->total_used_memory -= it->second->size;
   it->second->info->total_used_memory += new_size;

   if (old_ptr == new_ptr) {
      it->second->size = new_size;
   } else {
      memory_info_map.erase(it);
      auto _memory_info = std::make_unique<account_memory_info>();
      _memory_info->size = new_size;
      _memory_info->info = _account_info->second;
      memory_info_map[new_ptr] = std::move(_memory_info);
   }
}

void inspector::memory_trace_free(void* ptr) {
   if (!current_account) {
      return;
   }
//   printf("++++++++++memory_trace_free %p\n", ptr);
   auto it = memory_info_map.find(ptr);
   if (it == memory_info_map.end()) {
      return;
   }
   it->second->info->total_used_memory -= it->second->size;
//   printf("+++memory_trace_free: total_used_memory %lld %d \n", it->second->info->account, it->second->info->total_used_memory);
   memory_info_map.erase(it);
}

int inspector::inspect_memory() {
   if (!current_account) {
      return 1;
   }
   auto _account_info = accounts_info_map.find(current_account);
   if (_account_info == accounts_info_map.end()) {
      return 1;
   }
//   printf("++++++_account_info->second->total_used_memory %lld %d \n", current_account, _account_info->second->total_used_memory);
   return _account_info->second->total_used_memory <=500*1024;
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

int inspect_setattr_(PyObject* v, PyObject* name) {
   return inspector::get().inspect_setattr(v, name);
}

int inspect_getattr_(PyObject* v, PyObject* name) {
   return inspector::get().inspect_getattr(v, name);
}

int whitelist_opcode_(int opcode) {
   return inspector::get().whitelist_opcode(opcode);
}


int inspect_opcode_(int opcode) {
   return inspector::get().inspect_opcode(opcode);
}

int inspect_build_class_(PyObject* cls) {
   return inspector::get().inspect_build_class(cls);
}

int is_class_in_current_account_(PyObject* obj) {
   PyObject* cls = PyObject_GetAttrString(obj, "__class__");
   return inspector::get().is_class_in_current_account(cls);
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

void enable_filter_set_attr_(int enable) {
   if (enable) {
      get_injected_apis()->enable_filter_set_attr = 1;
   } else {
      get_injected_apis()->enable_filter_set_attr = 0;
   }
}

void enable_filter_get_attr_(int enable) {
   if (enable) {
      get_injected_apis()->enable_filter_get_attr = 1;
   } else {
      get_injected_apis()->enable_filter_get_attr = 0;
   }
}

void set_current_account_(uint64_t account) {
   inspector::get().set_current_account(account);
}

void set_current_module_(PyObject* mod) {
   inspector::get().set_current_module(mod);
}

void add_code_object_to_current_account_(PyCodeObject* co) {
   inspector::get().add_code_object_to_current_account(co);
}

int is_code_object_in_current_account_(PyCodeObject* co) {
   return inspector::get().is_code_object_in_current_account(co);
}

void memory_trace_start_() {
   inspector::get().memory_trace_start();
}

void memory_trace_stop_() {
   inspector::get().memory_trace_stop();
}

void memory_trace_alloc_(void* ptr, size_t new_size) {
   inspector::get().memory_trace_alloc(ptr, new_size);
}

void memory_trace_realloc_(void* old_ptr, void* new_ptr, size_t new_size) {
   inspector::get().memory_trace_realloc(old_ptr, new_ptr, new_size);
}

void memory_trace_free_(void* ptr) {
   inspector::get().memory_trace_free(ptr);
}

int inspect_memory_() {
   return inspector::get().inspect_memory();
}

int inspect_obj_creation_(PyTypeObject* type) {
   return inspector::get().inspect_obj_creation(type);
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
   apis->enable_filter_set_attr = 0;
   apis->enable_filter_get_attr = 0;
   apis->enable_inspect_obj_creation = 0;

   apis->whitelist_function = whitelist_function_;
   apis->inspect_function = inspect_function_;

   apis->whitelist_import_name = whitelist_import_name_;
   apis->inspect_import_name = inspect_import_name_;


   apis->whitelist_attr = whitelist_attr_;
   apis->inspect_setattr = inspect_setattr_;
   apis->inspect_getattr = inspect_getattr_;


   apis->inspect_opcode = inspect_opcode_;

   apis->inspect_build_class = inspect_build_class_;

   apis->memory_trace_start = memory_trace_start_;
   apis->memory_trace_stop = memory_trace_stop_;
   apis->memory_trace_alloc = memory_trace_alloc_;
   apis->memory_trace_realloc = memory_trace_realloc_;
   apis->memory_trace_free = memory_trace_free_;

   apis->inspect_memory = inspect_memory_;

   apis->inspect_obj_creation = inspect_obj_creation_;

   apis->check_time = check_time_;

   apis->add_code_object_to_current_account = add_code_object_to_current_account_;
}
