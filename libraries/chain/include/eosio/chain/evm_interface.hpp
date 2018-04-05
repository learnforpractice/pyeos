#pragma once


#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/types.hpp>
#include <eosio/chain/apply_context.hpp>

#include <Runtime/Runtime.h>
#include "IR/Module.h"

#include <libethcore/SealEngine.h>

using namespace std;
using namespace dev::eth;
class SealEngineFace;

namespace eosio {
namespace chain {

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
   void init();
   static evm_interface& get();
   void apply(apply_context& c, const shared_vector<char>&  code);

   bool run_code(apply_context& context, bytes& code, bytes& data, eosio::chain::bytes& output);

   apply_context* current_apply_context = nullptr;

   std::unique_ptr<dev::eth::SealEngineFace> se;

private:
   evm_interface();


};

}
} // eos::chain

void evm_test_(string _code, string _data);
