#include <eosio/chain/webassembly/runtime_interface.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <fstream>

#include <fc/crypto/xxhash.h>
#include <eosio/chain/wasm_interface_private.hpp>
#include <eosio/chain/wasm_eosio_validation.hpp>

#include <eosiolib/system.h>

using namespace std;

namespace eosio { namespace chain {
   using namespace webassembly;
   using namespace webassembly::common;

   wasm_interface& wasm_interface::get() {
      static wasm_interface* instance = nullptr;
      if (!instance) {
         instance = new wasm_interface(wasm_interface::vm_type::wabt);
      }
      return *instance;
   }

   wasm_interface::wasm_interface(vm_type vm) : my( new wasm_interface_impl(vm) ) {
   }

   wasm_interface::~wasm_interface() {}

   bool wasm_interface::init() {
      return true;
   }
   void wasm_interface::validate(const char* code, size_t size) {
      Module module;
      try {
         Serialization::MemoryInputStream stream((U8*)code, size);
         WASM::serialize(stream, module);
//         module.userSections.clear();
      } catch(const Serialization::FatalSerializationException& e) {
         EOS_THROW(wasm_serialization_error, e.message.c_str());
      } catch(const IR::ValidationException& e) {
         EOS_THROW(wasm_serialization_error, e.message.c_str());
      }

      wasm_validations::wasm_binary_validation validator(get_vm_api()->is_producing_block(), module);
      validator.validate();

//      root_resolver resolver(true);
//      LinkResult link_result = linkModule(module, resolver);

      //there are a couple opportunties for improvement here--
      //Easy: Cache the Module created here so it can be reused for instantiaion
      //Hard: Kick off instantiation in a separate thread at this location
   }

   int wasm_interface::setcode( uint64_t account) {
      size_t size = 0;
      const char* code;
      code = get_code( account, &size );
      if (size <= 0) {
         return my->unload_module(account);
      }
      validate(code, size);

      my->get_instantiated_module(account);
      return 0;
   }

   uint64_t wasm_interface::call(string& func, vector<uint64_t>& args) {
      try {
         uint64_t receiver = get_vm_api()->current_receiver();
         auto& module = my->get_instantiated_module(receiver);
         if (!module.get()) {
            return VM_MODULE_NOT_FOUND;
         }
         module->call(func, args);
      } catch ( const wasm_exit& ){
      }
      return 1;
   }

   int wasm_interface::apply( uint64_t receiver, uint64_t account, uint64_t act ) {
      try {
         auto& module = my->get_instantiated_module(receiver);
         if (!module.get()) {
            return VM_MODULE_NOT_FOUND;
         }
         module->apply(receiver, account, act);
      } catch ( const wasm_exit& ){
      }
      return 1;
   }

   int wasm_interface::preload(uint64_t account) {
      my->get_instantiated_module(account, true);
      return 1;
   }

   int wasm_interface::unload(uint64_t account) {
      return my->unload_module(account);
   }

   wasm_instantiated_module_interface::~wasm_instantiated_module_interface() {}
   wasm_runtime_interface::~wasm_runtime_interface() {}

} } /// eosio::chain

using namespace eosio::chain;

int wasm_setcode(uint64_t account) {
   return wasm_interface::get().setcode(account);
}

int wasm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   return wasm_interface::get().apply(receiver, account, act);
}

int wasm_preload(uint64_t account) {
   return wasm_interface::get().preload(account);
}

int wasm_unload(uint64_t account) {
   return wasm_interface::get().unload(account);
}

uint64_t _wasm_call(const char* act, uint64_t* args, int argc) {
   vector<uint64_t> v;
   for (int i=0;i<argc;i++) {
      v.push_back(args[i]);
   }
   string _act = string(act);
   return wasm_interface::get().call(_act, v);
}

