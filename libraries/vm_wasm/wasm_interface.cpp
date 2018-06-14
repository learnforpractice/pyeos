#include <eosio/chain/wasm_interface_private.hpp>
#include <eosio/chain/webassembly/runtime_interface.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <fstream>

#include <fc/crypto/xxhash.h>

using namespace std;

namespace eosio { namespace chain {
   using namespace webassembly;
   using namespace webassembly::common;

   wasm_interface& wasm_interface::get() {
      static wasm_interface* instance = nullptr;
      if (!instance) {
         instance = new wasm_interface(wasm_interface::vm_type::binaryen);
      }
      return *instance;
   }

   wasm_interface::wasm_interface(vm_type vm) : my( new wasm_interface_impl(vm) ) {
   }

   wasm_interface::~wasm_interface() {}

   bool wasm_interface::init() {
      return true;
   }

   int wasm_interface::setcode( uint64_t account) {
      return 0;
   }

   int wasm_interface::apply( uint64_t receiver, uint64_t account, uint64_t act ) {
      try {
         my->get_instantiated_module(receiver)->apply(receiver, account, act);
      } catch ( const wasm_exit& ){
      }
      return 1;
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

