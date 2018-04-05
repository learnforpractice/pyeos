#include <boost/function.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/thread/thread.hpp>
#include <eosio/chain/micropython_interface.hpp>
#include <eosio/chain/chain_controller.hpp>
#include "Platform/Platform.h"
#include "WAST/WAST.h"
#include "Runtime/Runtime.h"
#include "Runtime/Linker.h"
#include "Runtime/Intrinsics.h"
#include "IR/Module.h"
#include "IR/Operators.h"
#include "IR/Validate.h"
#include <eosio/chain/account_object.hpp>
#include <chrono>
#include <appbase/application.hpp>

extern "C" {
   mp_obj_t micropy_load_from_py(const char *mod_name, const char *data, size_t len);
   mp_obj_t micropy_load_from_mpy(const char *mod_name, const char *data, size_t len);
   mp_obj_t micropy_call_0(mp_obj_t module_obj, const char *func);
   mp_obj_t micropy_call_2(mp_obj_t module_obj, const char *func, uint64_t code, uint64_t type);
}




#if 0
extern "C" void print_time() {
   ilog("fc::time_point::microseconds() ${n}", ("n", fc::time_point::now()));
}
#endif

namespace eosio {
namespace chain {

static apply_context* s_context(0);

void set_current_context(apply_context* context) {
   s_context = context;
}

apply_context* get_current_context() {
   return s_context;
}

bool get_code(uint64_t _account, std::vector<uint8_t>& v) {
   return s_context->get_code(_account, v);
}
bool get_code_size(uint64_t _account, int& size) {
   return s_context->get_code_size(_account, size);
}

micropython_interface::micropython_interface() {
}

micropython_interface& micropython_interface::get() {
   static micropython_interface* python = nullptr;
   if (!python) {
      wlog("micropython_interface::init");
      python = new micropython_interface();
   }
   return *python;
}

void micropython_interface::on_setcode(uint64_t _account, bytes& code) {
    auto itr = module_cache.find(_account);

   if (itr != module_cache.end()) {
      fc::sha256 hash = fc::sha256::hash( code.data(), code.size() );
      //FIXME: handle hash conflict
      if (itr->second->hash == hash) {
         return;
      }
   }
   ilog("++++++++++update code");
   mp_obj_t obj = nullptr;
   if (code.data()[0] == 0) {//py
      obj = micropy_load_from_py(name(_account).to_string().c_str(), (const char*)&code.data()[1], code.size()-1);
   } else if (code.data()[0] == 1) {//mpy
      obj = micropy_load_from_mpy(name(_account).to_string().c_str(), (const char*)&code.data()[1], code.size()-1);
   } else {
      FC_ASSERT(false, "unknown micropython code!");
   }
   if (obj != NULL) {
      py_module* mod = new py_module();
      mod->obj = obj;
      mod->hash = fc::sha256::hash( code.data(), code.size() );
      module_cache[_account] = mod;
   }
}

void micropython_interface::apply(apply_context& c, const shared_vector<char>& code) {
      try {
      set_current_context(&c);
      current_apply_context = &c;
      mp_obj_t obj = nullptr;
      nlr_buf_t nlr;
      if (nlr_push(&nlr) == 0) {
         auto itr = module_cache.find(c.act.account.value);
         if (itr != module_cache.end()) {
            obj = itr->second->obj;
         } else {
            if (code.data()[0] == 0) {//py
               obj = micropy_load_from_py(c.act.account.to_string().c_str(), (const char*)&code.data()[1], code.size()-1);
            } else if (code.data()[0] == 1) {//mpy
               obj = micropy_load_from_mpy(c.act.account.to_string().c_str(), (const char*)&code.data()[1], code.size()-1);
            } else {
               FC_ASSERT(false, "unknown micropython code!");
            }
            if (obj != NULL) {
               py_module* mod = new py_module();
               mod->obj = obj;
               mod->hash = fc::sha256::hash( code.data(), code.size() );
               module_cache[c.act.account.value] = mod;
            }
         }
         if (obj) {
            micropy_call_2(obj, "apply", c.act.account.value, c.act.name.value);
         }
         nlr_pop();
      } else {
         mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));
         throw fc::exception();
         // uncaught exception
         //          return (mp_obj_t)nlr.ret_val;
      }
   }FC_CAPTURE_AND_RETHROW()
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
       mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));
        // uncaught exception
        return (mp_obj_t)nlr.ret_val;
    }
}
