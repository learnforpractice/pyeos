#pragma once

extern "C" {
#include <stdio.h>
#include <string.h>

#include "py/objlist.h"
#include "py/objstringio.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/obj.h"
#include "py/compile.h"
}

#include <thread>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/types.hpp>

#include <Runtime/Runtime.h>
#include "IR/Module.h"

#include <fc/crypto/sha256.hpp>

using namespace std;

namespace eosio {
namespace chain {

class apply_context;
class chain_controller;

/**
 * @class micropython_interface
 *
 * EOS uses the wasm-jit library to evaluate web assembly code. This library relies
 * upon a singlton interface which means there can be only one instance. This interface
 * is designed to wrap that singlton interface and potentially make it thread-local state
 * in the future.
 */
class micropython_interface {
public:

   static micropython_interface& get();

   void on_setcode(uint64_t _account, bytes& code);
   void on_server_setcode(uint64_t _account, bytes& code);
   void on_client_setcode(uint64_t _account);

   void apply(uint64_t receiver, uint64_t account, uint64_t act, const shared_string& code);
   void apply(uint64_t receiver, uint64_t account, uint64_t act);
private:
   struct py_module {
      void* obj;
      fc::sha256 hash;
   };

   std::map<std::thread::id, std::map<uint64_t, py_module*>> module_cache;
   micropython_interface();
};


}
} // eos::chain

void* execute_from_str(const char *str);
