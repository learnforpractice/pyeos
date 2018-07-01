#include <eosio/chain/wasm_interface.hpp>
#include <eosio/chain/apply_context.hpp>
#include <eosio/chain/controller.hpp>
#include <eosio/chain/transaction_context.hpp>
#include <eosio/chain/producer_schedule.hpp>
#include <eosio/chain/exceptions.hpp>
#include <boost/core/ignore_unused.hpp>
#include <eosio/chain/authorization_manager.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <eosio/chain/wasm_interface_private.hpp>
//#include <eosio/chain/wasm_eosio_validation.hpp>
//#include <eosio/chain/wasm_eosio_injection.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/account_object.hpp>
#include <eosio/chain/symbol.hpp>

#include <fc/exception/exception.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/sha1.hpp>
#include <fc/io/raw.hpp>

#include <softfloat.hpp>
#include <compiler_builtins.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <fstream>

#include <fc/crypto/xxhash.h>

#include "vm_manager.hpp"


static bool _wasm_debug_enable = 0;
static bool _enable_native_contract = true;

void wasm_debug_enable_(int enable) {
   _wasm_debug_enable = enable;
}

bool wasm_debug_enabled_() {
   return _wasm_debug_enable;
}

void wasm_enable_native_contract_(bool b) {
   _enable_native_contract = b;
}

bool wasm_is_native_contract_enabled_() {
   return _enable_native_contract;
}

static string debug_contract_path;
static uint64_t debug_account;

void set_debug_contract_(string& _account, string& path) {
   debug_account = eosio::chain::string_to_name(_account.c_str());
   debug_contract_path = path;
}

namespace eosio { namespace chain {

   void register_vm_api(void* handle);
   void wasm_init_api();

    static fn_apply _old_eosio_apply = nullptr;
    void load_system_contract_v1() {
        if (_old_eosio_apply) {
            return;
        }
        
        char _path[128];
        void* handle;
        snprintf(_path, sizeof(_path), "../libs/libeosio_native_v1%s", DYLIB_SUFFIX);
        handle = dlopen(_path, RTLD_LAZY | RTLD_LOCAL);
        if (!handle) {
            snprintf(_path, sizeof(_path), "../libs/libeosio_native_v1d%s", DYLIB_SUFFIX);
            handle = dlopen(_path, RTLD_LAZY | RTLD_LOCAL);
            if (!handle) {
                return;
            }
        }
        
        register_vm_api(handle);
        _old_eosio_apply = (fn_apply)dlsym(handle, "apply");
    }
    
   wasm_interface::wasm_interface(vm_type vm)  : my( new wasm_interface_impl(vm) ) {
       load_system_contract_v1();
   }

   wasm_interface::~wasm_interface() {}

   bool wasm_interface::init() {
      my->init_native_contract();
//      wasm_init_api();
      return true;
   }

   void wasm_interface::validate(const controller& control, const bytes& code) {

   }

   void wasm_interface::call( const digest_type& code_id, const shared_string& code, string& func, vector<uint64_t>& args, apply_context& context ) {
   //      my->get_instantiated_module(context.act.account, code_id, code)->call(func, args, context);
   //      my->get_instantiated_module(code_id, code, context.trx_context)->call(func, args, context);
   }

   bool wasm_interface::apply_native(apply_context& ctx) {
      string contract_path;
      uint64_t native = N(native);
      fn_apply _apply = nullptr;

      if (!_enable_native_contract) {
         return false;
      }

      if (debug_account == ctx.act.account.value) {
         void *handle;
         contract_path = debug_contract_path;

         handle = dlopen(contract_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
         if (!handle) {
            elog("open dll ${n} failed", ("n", contract_path));
            return false;
         }

         register_vm_api(handle);
         fn_apply _apply = (fn_apply)dlsym(handle, "apply");
         if (!_apply) {
            elog("apply not found in ${n}", ("n", contract_path));
            return false;
         }
         _apply(ctx.receiver, ctx.act.account, ctx.act.name);
         return true;
      }

      auto itr = my->native_cache.find(ctx.act.account.value);
      if (itr == my->native_cache.end()) {
         _apply = my->load_native_contract(ctx.act.account.value);
         if (!_apply) {
            return false;
         }
      } else {
         char _name[64];
         snprintf(_name, sizeof(_name), "%s.%d", ctx.act.account.to_string().c_str(), NATIVE_PLATFORM);
         uint64_t __account = NN(_name);

         int _itr = db_api::get().db_find_i64(native, native, native, __account);
         if (_itr < 0) {
            _apply = itr->second->apply;
         } else {
            size_t buffer_size = 0;
            const char* code = db_api::get().db_get_i64_exex(_itr, &buffer_size);
            uint32_t version = *(uint32_t*)code;
            if (version > itr->second->version) {
                wlog("reloading native contract ${n} ${n2} ${n3}", ("n", ctx.act.account.to_string())("n2", version)("n3",itr->second->version));
               _apply = my->load_native_contract(ctx.act.account.value);
               if (!_apply) {
                  return false;
               }
            } else {
               _apply = itr->second->apply;
            }
         }
      }

      _apply(ctx.receiver, ctx.act.account, ctx.act.name);
      return true;
   }

   void wasm_interface::apply( const digest_type& code_id, const shared_string& code, apply_context& context ) {
      uint32_t num = context.control.head_block_state()->header.block_num();
      while (true) {//(num >= 200) {
         if (context.receiver == N(eosio)) {
            my->_eosio_apply(context.receiver.value, context.act.account.value, context.act.name.value);
//            _old_eosio_apply(context.receiver.value, context.act.account.value, context.act.name.value);
            return;
              //context.act.name == N(buyrambytes) || context.act.name == N(buyram) || context.act.name == N(sellram)
//            12000 setcode eosio set bios.boot
//            12247 setcode eosio set back eosio.system
            if (num  <= 11999) {
               _old_eosio_apply(context.receiver.value, context.act.account.value, context.act.name.value);
               return;
            }
            if (num >= 12248) {
               _old_eosio_apply(context.receiver.value, context.act.account.value, context.act.name.value);
               return;
            }
            break;
         }

         if (context.receiver == N(eosio.token) && apply_native(context)) {
            return;
         }
         break;
      }

      vm_manager::get().apply(0, context.receiver.value, context.act.account.value, context.act.name.value);
   }

} } /// eosio::chain
