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


#include <eos/chain/exceptions.hpp>
#include <eos/chain/message.hpp>
#include <eos/chain/message_handling_contexts.hpp>
#include <Runtime/Runtime.h>
#include "IR/Module.h"


using namespace std;

namespace eosio {
namespace chain {

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
   struct ModuleState {
      Runtime::ModuleInstance* instance = nullptr;
      IR::Module* module = nullptr;
      int mem_start = 0;
      int mem_end = 1 << 16;
      vector<char> init_memory;
      fc::sha256 code_version;
   };

   static micropython_interface& get();

   void init(apply_context& c);
   void apply(apply_context& c);
   void validate(apply_context& c);
   void precondition(apply_context& c);

   int64_t current_execution_time();

   void set_current_context(apply_context& c);

   apply_context* current_apply_context = nullptr;
   apply_context* current_validate_context = nullptr;
   apply_context* current_precondition_context = nullptr;

   Runtime::MemoryInstance* current_memory = nullptr;
//      Runtime::ModuleInstance*   current_module  = nullptr;
   string current_module;
   ModuleState* current_state = nullptr;
   class mp_py_module {
   public:
      mp_py_module(mp_obj_t obj, string& src) {
         this->obj = obj;
         this->src = src;
      };
      mp_obj_t obj;
      string src;
   };
   std::map<std::string, mp_py_module*> py_modules;

   mp_obj_t current_py_module;

private:
   void load(const account_name& name, const chainbase::database& db);

   char* vm_allocate(int bytes);
   void vm_call(const char* name);
   void vm_validate();
   void vm_precondition();
   void vm_apply();
   void vm_onInit();
   U32 vm_pointer_to_offset(char*);

   map<account_name, ModuleState> instances;
   fc::time_point checktimeStart;

   micropython_interface();
};

}
} // eos::chain

void* execute_from_str(const char *str);
