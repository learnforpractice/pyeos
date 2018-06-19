#include <boost/function.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/thread/thread.hpp>
#include <eosio/chain/micropython_interface.hpp>
#include <eosio/chain/controller.hpp>
#include <eosio/chain/account_object.hpp>
#include <eosio/chain/apply_context.hpp>

#include "Platform/Platform.h"
#include "WAST/WAST.h"
#include "Runtime/Runtime.h"
#include "Runtime/Linker.h"
#include "Runtime/Intrinsics.h"
#include "IR/Module.h"
#include "IR/Operators.h"
#include "IR/Validate.h"
#include <chrono>
#include <appbase/application.hpp>
#include <fc/crypto/xxhash.h>
#include "micropython/db_api.hpp"

#include "micropython/mpeoslib.h"
//#include "rpc_interface/rpc_interface.hpp"

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

// 'storage.py'
const char * init_mp = "" \
"libs = ('asset.py', 'token.py', 'cache.py', 'garden.py', 'solidity.py', 'storage.py')\n" \
"for lib in libs:\n" \
"    mod = 'backyard.'+lib[:-3]\n" \
"    print(mod)\n"
"    __import__(mod)\n"
;

void init() {
   if (get_mpapi().init) {
      return;
   }
   wlog("Initialize common library.");

   uint64_t hash = XXH64("storage.mpy", strlen("storage.mpy"), 0);

   int itr = db_api::get().db_find_i64(N(backyard), N(backyard), N(backyard), hash);
   if (itr < 0) {
      return;
   }
   wlog("Execute script");
   get_mpapi().execute_from_str(init_mp);
   get_mpapi().init = 1;
}

void micropython_interface::on_server_setcode(uint64_t _account, bytes& code) {
#if 0
   if (rpc_interface::get().ready()) {
      rpc_interface::get().on_setcode(_account, code);
      return;
   }
#endif
}

void micropython_interface::on_client_setcode(uint64_t _account) {
   string code;
   db_api::get().get_code(_account, code);
   vector<char> _code(code.begin(), code.end());
   on_setcode(_account, _code);
}


void micropython_interface::on_setcode(uint64_t _account, bytes& code) {
   if (code.size() <= 0) {
      return;
   }

   if (appbase::app().client_mode()) {
      //called by client
   } else {
      //called by server
      if (!db_api::get().is_in_whitelist(_account)) {
#if 0
         if (rpc_interface::get().ready()) {
            rpc_interface::get().on_setcode(_account, code);
            return;
         }
#endif
         //FC_ASSERT(false, "RPC not ready");
      }
   }

   init();

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
   mp_raw_code_t* raw_code = nullptr;
   get_mpapi().execution_start();
   if (code.data()[0] == 0) {//py
      obj = get_mpapi().micropy_load_from_py(name(_account).to_string().c_str(), (const char*)&code.data()[1], code.size()-1);
   } else if (code.data()[0] == 1) {//mpy
      obj = get_mpapi().micropy_load_from_mpy(name(_account).to_string().c_str(), (const char*)&code.data()[1], code.size()-1, &raw_code);
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
      mod->raw_code = raw_code;
      mod->hash = fc::sha256::hash( code.data(), code.size() );
      pymodules[_account] = mod;
   } else {
      FC_ASSERT(false, "load micropython code failed!");
   }
}

void micropython_interface::apply(uint64_t receiver, uint64_t account, uint64_t act, const shared_string& code) {
   init();
   get_mpapi().execution_start();

   try {
      mp_obj_t obj = nullptr;
      mp_raw_code_t* raw_code = nullptr;
      nlr_buf_t nlr;

      std::thread::id this_id = std::this_thread::get_id();

      auto _itr = module_cache.find(this_id);
      if (_itr == module_cache.end()) {
         module_cache[this_id] = std::map<uint64_t, py_module*>();
      }

      std::map<uint64_t, py_module*>& pymodules = module_cache[this_id];

      auto itr = pymodules.find(receiver);
      if (itr != pymodules.end()) {
         obj = itr->second->obj;
         raw_code = itr->second->raw_code;
      } else {
         if (code.data()[0] == 0) {//py
            obj = get_mpapi().micropy_load_from_py(account_name(account).to_string().c_str(), (const char*)&code.data()[1], code.size()-1);
         } else if (code.data()[0] == 1) {//mpy
            obj = get_mpapi().micropy_load_from_mpy(account_name(account).to_string().c_str(), (const char*)&code.data()[1], code.size()-1, &raw_code);
         } else {
            FC_ASSERT(false, "unknown micropython code!");
         }

         if (obj != NULL) {
            py_module* mod = new py_module();
            mod->obj = obj;
            mod->raw_code = raw_code;
            mod->hash = fc::sha256::hash( code.data(), code.size() );
            pymodules[account] = mod;
         }
      }
      mp_obj_t ret = 0;
      if (obj) {
         ret = get_mpapi().micropy_call_3(obj, raw_code, "apply", receiver, account, act);
      }
      uint64_t execution_time = get_mpapi().get_execution_time();
      if (execution_time > 1000) {
         elog("+++++++call module ${n1}, cost: ${n2}", ("n1", name(account).to_string())("n2", execution_time));
      }
      get_mpapi().execution_end();
      FC_ASSERT(ret != 0, "code execution with exception!");
   } catch(...) {
      get_mpapi().execution_end();
      throw;
   }

}

void micropython_interface::apply(uint64_t receiver, uint64_t account, uint64_t act) {
   const shared_string& src = db_api::get().get_code(receiver);
   apply(receiver, account, act, src);
}

}
}
using namespace eosio::chain;
//called by rpc client
extern "C" int micropython_on_apply(uint64_t receiver, uint64_t account, uint64_t act, char** err) {
   string _err;
   try {
      if (account == N(eosio) && act == N(setcode)) {
         micropython_interface::get().on_client_setcode(receiver);
      } else {
         micropython_interface::get().apply(receiver, account, act);
      }
      return 0;
   } catch (fc::assert_exception& e) {
      _err = e.to_detail_string();
      elog(_err);
   } catch (fc::exception& e) {
      _err = e.to_detail_string();
      elog(_err);
   } catch (boost::exception& ex) {
      _err = boost::diagnostic_information(ex);
      elog(_err);
   }
   if (!_err.empty()) {
      wlog(_err);
      *err = (char*)calloc(_err.length()+1, sizeof(char));
      memcpy(*err, _err.c_str(), _err.length());
   }
   return 1;
}


