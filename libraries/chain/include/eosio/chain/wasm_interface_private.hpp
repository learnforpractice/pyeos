#pragma once

#include <eosio/chain/wasm_interface.hpp>
#include <eosio/chain/transaction_context.hpp>
#include <fc/scoped_exit.hpp>

#include <dlfcn.h>

#include "micropython/db_api.hpp"

using namespace fc;

#if defined(__APPLE__) && defined(__MACH__)
   #define NATIVE_PLATFORM 1
#elif defined(__linux__)
   #define NATIVE_PLATFORM 2
#elif defined(_WIN64)
   #define NATIVE_PLATFORM 3
#else
   #error Not Supported Platform
#endif

namespace eosio { namespace chain {
   void register_vm_api(void* handle);
   typedef void (*fn_apply)(uint64_t receiver, uint64_t account, uint64_t act);

   struct native_code_cache {
         uint32_t version;
         void *handle;
         fn_apply apply;
   };

   struct wasm_interface_impl {
      wasm_interface_impl(wasm_interface::vm_type vm) {

      }

      void init_native_contract() {
         uint64_t native_account[] = {N(eosio.bios), N(eosio.msig), N(eosio.token), N(eosio)/*eosio.system*/, N(exchange)};
         for (int i=0; i<sizeof(native_account)/sizeof(native_account[0]); i++) {
            load_native_contract(native_account[i]);
         }
      }

      fn_apply load_native_contract(uint64_t _account) {
         string contract_path;
         uint64_t native = N(native);
         void *handle = nullptr;
         char _name[64];
         snprintf(_name, sizeof(_name), "%s.%d", name(_account).to_string().c_str(), NATIVE_PLATFORM);
         uint64_t __account = NN(_name);

         int itr = db_api::get().db_find_i64(native, native, native, __account);
         if (itr < 0) {
            return nullptr;
         }

         size_t native_size = 0;
         const char* code = db_api::get().db_get_i64_exex(itr, &native_size);
         uint32_t version = *(uint32_t*)code;

         char native_path[64];
         sprintf(native_path, "%s.%d",name(__account).to_string().c_str(), version);

         wlog("loading native contract:\t ${n}", ("n", native_path));

         struct stat _s;
         if (stat(native_path, &_s) == 0) {
            //
         } else {
            std::ofstream out(native_path, std::ios::binary | std::ios::out);
            out.write(&code[4], native_size - 4);
            out.close();
         }
         contract_path = native_path;

         handle = dlopen(contract_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
         if (!handle) {
            return nullptr;
         }
         register_vm_api(handle);
         fn_apply _apply = (fn_apply)dlsym(handle, "apply");

         std::unique_ptr<native_code_cache> _cache = std::make_unique<native_code_cache>();
         _cache->version = version;
         _cache->handle = handle;
         _cache->apply = _apply;
   //      native_cache.emplace(_account, std::move(_cache));
         native_cache[_account] =  std::move(_cache);
         return _apply;
      }

      map<uint64_t, std::unique_ptr<native_code_cache>> native_cache;
   };

} } // eosio::chain
