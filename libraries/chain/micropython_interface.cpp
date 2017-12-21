#include <boost/function.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/thread/thread.hpp>
#include <eos/chain/micropython_interface.hpp>
#include <eos/chain/wasm_interface.hpp>
#include <eos/chain/chain_controller.hpp>
#include "Platform/Platform.h"
#include "WAST/WAST.h"
#include "Runtime/Runtime.h"
#include "Runtime/Linker.h"
#include "Runtime/Intrinsics.h"
#include "IR/Module.h"
#include "IR/Operators.h"
#include "IR/Validate.h"
#include <eos/chain/key_value_object.hpp>
#include <eos/chain/account_object.hpp>
#include <chrono>
#include <appbase/application.hpp>


extern "C" {
   mp_obj_t micropy_load(const char *mod_name, const char *data, size_t len);
   mp_obj_t micropy_call_0(mp_obj_t module_obj, const char *func);
   mp_obj_t micropy_call_2(mp_obj_t module_obj, const char *func, uint64_t code, uint64_t type);
}

namespace eosio {
namespace chain {
using namespace IR;
using namespace Runtime;
typedef boost::multiprecision::cpp_bin_float_50 DOUBLE;


micropython_interface::micropython_interface() {
}

template<typename Function, typename KeyType, int numberOfKeys>
int32_t validate(int32_t valueptr, int32_t valuelen, Function func) {

   static const uint32_t keylen = numberOfKeys * sizeof(KeyType);

   FC_ASSERT(valuelen >= keylen, "insufficient data passed");

   auto& wasm = micropython_interface::get();
   FC_ASSERT(wasm.current_apply_context, "no apply context found");

   char* value = memoryArrayPtr<char>(wasm.current_memory, valueptr, valuelen);
   KeyType* keys = reinterpret_cast<KeyType*>(value);

   valuelen -= keylen;
   value += keylen;

   return func(wasm.current_apply_context, keys, value, valuelen);
}

micropython_interface& micropython_interface::get() {
   static micropython_interface* python = nullptr;
   if (!python) {
      wlog("micropython_interface::init");
      python = new micropython_interface();
   }
   return *python;
}

int64_t micropython_interface::current_execution_time() {
   return (fc::time_point::now() - checktimeStart).count();
}

char* micropython_interface::vm_allocate(int bytes) {
   return 0;
}

U32 micropython_interface::vm_pointer_to_offset(char* ptr) {
   return U32(ptr - &memoryRef<char>(current_memory, 0));
}

void micropython_interface::vm_call(const char* function_name) {
   micropy_call_2(current_py_module, function_name, current_validate_context->msg.code.value, current_validate_context->msg.type.value);
}

void micropython_interface::vm_apply() {
   vm_call("apply");
}

void micropython_interface::vm_onInit() {
   vector<uint64_t> args;
   string module_name = current_module;
   string function_name = "init";
   if (current_py_module != NULL) {
      micropy_call_0(current_py_module, "init");
   }
   //   python_call_with_gil(module_name, function_name, args);
}

void micropython_interface::validate(apply_context& c) {
   /*
    current_validate_context       = &c;
    current_precondition_context   = nullptr;
    current_apply_context          = nullptr;

    load( c.code, c.db );
    vm_validate();
    */
}

void micropython_interface::precondition(apply_context& c) {
   try {

      /*
       current_validate_context       = &c;
       current_precondition_context   = &c;

       load( c.code, c.db );
       vm_precondition();
       */

   }FC_CAPTURE_AND_RETHROW()
}

void micropython_interface::apply(apply_context& c) {
   try {
      current_validate_context = &c;
      current_precondition_context = &c;
      current_apply_context = &c;

      load(c.code, c.db);
      vm_apply();

   }FC_CAPTURE_AND_RETHROW()
}

void micropython_interface::init(apply_context& c) {
   try {
      current_validate_context = &c;
      current_precondition_context = &c;
      current_apply_context = &c;
      load(c.code, c.db);
      vm_onInit();
   }FC_CAPTURE_AND_RETHROW()
}

void micropython_interface::load(const account_name& _name, const chainbase::database& db) {
   const auto& recipient = db.get<account_object, by_name>(_name);
   string module_name = string(name(_name));
   string code = string((const char*) recipient.code.data(), recipient.code.size());
   current_module = module_name;

   ilog("micropython_interface::load");

   std::map<string, mp_py_module*>::iterator it = py_modules.find(module_name);
   if (it == py_modules.end() || it->second->src != recipient.code.data()) {
      ilog("code : ${n}",("n", code));
      mp_obj_t obj = micropy_load(module_name.c_str(), (const char*)recipient.code.data(), recipient.code.size());
      ilog("${n1}, obj ${n2}",("n1", module_name)("n2", (uint64_t)obj));
      if (obj != 0) {
         if (it == py_modules.end()) {
            ilog("888");
            string code(recipient.code.data(), recipient.code.size());
            mp_py_module *module = new mp_py_module(obj, code);
            ilog("111");
            py_modules[module_name] = module;
            ilog("222");
         } else {
            ilog("333");
            py_modules[module_name]->src = recipient.code.data();
            ilog("444");
            py_modules[module_name]->obj = obj;
         }
         current_py_module = obj;
      } else {
         py_modules[module_name] = 0;
         current_py_module = 0;
      }
   } else {
      current_py_module = py_modules[module_name]->obj;
   }

   ilog("micropython_interface::load end");

   //   ilog("micropython_interface::load");
//   python_load_with_gil(module_name, code);
}

void micropython_interface::set_current_context(apply_context& c) {
   current_validate_context = &c;
   current_precondition_context = &c;
   current_apply_context = &c;
}

}
}


void* execute_from_str(const char *str) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_lexer_t *lex = mp_lexer_new_from_str_len(0/*MP_QSTR_*/, str, strlen(str), false);
        mp_parse_tree_t pt = mp_parse(lex, MP_PARSE_FILE_INPUT);
        mp_obj_t module_fun = mp_compile(&pt, lex->source_name, MP_EMIT_OPT_NONE, false);
        mp_call_function_0(module_fun);
        nlr_pop();
        return 0;
    } else {
        // uncaught exception
        return (mp_obj_t)nlr.ret_val;
    }
}
