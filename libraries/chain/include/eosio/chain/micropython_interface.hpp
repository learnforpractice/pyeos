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
   void apply(apply_context& c, const shared_vector<char>&  code);

   apply_context* current_apply_context = nullptr;

private:

   micropython_interface();
};

apply_context* get_current_context();

}
} // eos::chain

void* execute_from_str(const char *str);
