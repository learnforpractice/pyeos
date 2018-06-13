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

   wasm_interface::wasm_interface(vm_type vm) {
   }

   wasm_interface::~wasm_interface() {}

   bool wasm_interface::init() {
      return true;
   }

   void wasm_interface::apply( uint64_t receiver, uint64_t account, uint64_t act ) {
//      my->get_instantiated_module(code_id, code)->apply(receiver, account, act);
   }

   wasm_instantiated_module_interface::~wasm_instantiated_module_interface() {}
   wasm_runtime_interface::~wasm_runtime_interface() {}

} } /// eosio::chain
