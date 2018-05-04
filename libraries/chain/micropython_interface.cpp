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

#include "micropython/mpeoslib.h"

struct mpapi& get_mpapi();

#if 0
extern "C" void print_time() {
   ilog("fc::time_point::microseconds() ${n}", ("n", fc::time_point::now()));
}
#endif

namespace eosio {
namespace chain {


bool get_code(uint64_t _account, std::vector<uint8_t>& v) {
   return apply_context::ctx().get_code(_account, v);
}
bool get_code_size(uint64_t _account, int& size) {
   return apply_context::ctx().get_code_size(_account, size);
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
   if (code.size() <= 0) {
      throw;
   }

   std::thread::id this_id = std::this_thread::get_id();
   auto itr = module_cache.find(this_id);
   if (itr == module_cache.end()) {
      module_cache[this_id] = std::map<uint64_t, py_module*>();
   }
   std::map<uint64_t, py_module*>& pymodules = module_cache[this_id];


   auto _itr = pymodules.find(_account);

   if (_itr != pymodules.end()) {
      fc::sha256 hash = fc::sha256::hash( code.data(), code.size() );
      //FIXME: handle hash conflict
      if (_itr->second->hash == hash) {
         return;
      }
   }
//   ilog("++++++++++update code ${n1}", ("n1", name(_account).to_string()));
   mp_obj_t obj = nullptr;
   get_mpapi().execution_start();
   if (code.data()[0] == 0) {//py
      obj = get_mpapi().micropy_load_from_py(name(_account).to_string().c_str(), (const char*)&code.data()[1], code.size()-1);
   } else if (code.data()[0] == 1) {//mpy
      obj = get_mpapi().micropy_load_from_mpy(name(_account).to_string().c_str(), (const char*)&code.data()[1], code.size()-1);
   } else {
      FC_ASSERT(false, "unknown micropython code!");
   }

   uint64_t execution_time = get_mpapi().get_execution_time();
   if (execution_time > 1000) {
      elog("+++++++load module ${n1}, cost: ${n2}", ("n1", name(_account).to_string())("n2", execution_time));
   }

   get_mpapi().execution_end();

   if (obj != NULL) {
      if (_itr != pymodules.end()) {
         delete _itr->second;
      }
      py_module* mod = new py_module();
      mod->obj = obj;
      mod->hash = fc::sha256::hash( code.data(), code.size() );
      pymodules[_account] = mod;
   } else {
      elog("load micropython code failed!");
   }
}

void micropython_interface::apply(apply_context& c, const shared_vector<char>& code) {
   try {
      current_apply_context = &c;
      mp_obj_t obj = nullptr;
      nlr_buf_t nlr;

      std::thread::id this_id = std::this_thread::get_id();

      auto _itr = module_cache.find(this_id);
      if (_itr == module_cache.end()) {
         module_cache[this_id] = std::map<uint64_t, py_module*>();
      }

      std::map<uint64_t, py_module*>& pymodules = module_cache[this_id];

      auto itr = pymodules.find(c.receiver);
      if (itr != pymodules.end()) {
         obj = itr->second->obj;
      } else {
         get_mpapi().execution_start();

         if (code.data()[0] == 0) {//py
            obj = get_mpapi().micropy_load_from_py(c.act.account.to_string().c_str(), (const char*)&code.data()[1], code.size()-1);
         } else if (code.data()[0] == 1) {//mpy
            obj = get_mpapi().micropy_load_from_mpy(c.act.account.to_string().c_str(), (const char*)&code.data()[1], code.size()-1);
         } else {
            FC_ASSERT(false, "unknown micropython code!");
         }

         uint64_t execution_time = get_mpapi().get_execution_time();
         if (execution_time > 1000) {
            elog("+++++++load module ${n1}, cost: ${n2}", ("n1", c.act.account.to_string())("n2", execution_time));
         }
         get_mpapi().execution_end();

         if (obj != NULL) {
            py_module* mod = new py_module();
            mod->obj = obj;
            mod->hash = fc::sha256::hash( code.data(), code.size() );
            pymodules[c.act.account.value] = mod;
         }
      }
      if (obj) {
         get_mpapi().execution_start();
         get_mpapi().micropy_call_2(obj, "apply", c.act.account.value, c.act.name.value);
         uint64_t execution_time = get_mpapi().get_execution_time();
         if (execution_time > 1000) {
            elog("+++++++execute code in ${n1}, cost: ${n2}", ("n1", c.act.account.to_string())("n2", execution_time));
         }
         get_mpapi().execution_end();

      }
   }FC_CAPTURE_AND_RETHROW()
}

}
}

