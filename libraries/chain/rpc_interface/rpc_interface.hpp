#pragma once

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
 * @class rpc_interface
 *
 * EOS uses the wasm-jit library to evaluate web assembly code. This library relies
 * upon a singlton interface which means there can be only one instance. This interface
 * is designed to wrap that singlton interface and potentially make it thread-local state
 * in the future.
 */
class rpc_interface {
public:

   static rpc_interface& get();

   void on_setcode(uint64_t _account, bytes& code);
   void apply(apply_context& c);
   bool ready();

   apply_context* current_apply_context = nullptr;

private:
   struct py_module {
      void* obj;
      fc::sha256 hash;
   };
   std::map<uint64_t, py_module*> module_cache;
   rpc_interface();
};

}
} // eos::chain

//defined in pyeos/main.cpp
void init_eos();
void install_ctrl_c_handler();

typedef int (*fn_rpc_apply)(uint64_t receiver, uint64_t code, uint64_t act);
extern "C" void rpc_register_apply_call(fn_rpc_apply fn);
