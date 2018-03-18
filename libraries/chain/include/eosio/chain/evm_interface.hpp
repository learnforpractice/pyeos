#pragma once


#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/types.hpp>

#include <Runtime/Runtime.h>
#include "IR/Module.h"


using namespace std;

namespace eosio {
namespace chain {

class apply_context;
class chain_controller;

/**
 * @class evm_interface
 *
 * EOS uses the wasm-jit library to evaluate web assembly code. This library relies
 * upon a singlton interface which means there can be only one instance. This interface
 * is designed to wrap that singlton interface and potentially make it thread-local state
 * in the future.
 */
class evm_interface {
public:

   static evm_interface& get();
   void apply(apply_context& c, const shared_vector<char>&  code);

   apply_context* current_apply_context = nullptr;

private:

   evm_interface();
};

}
} // eos::chain

void evm_test_(string _code, string _data);
