#pragma once
#include <eos/chain/exceptions.hpp>
#include <eos/chain/message.hpp>
#include <eos/chain/message_handling_contexts.hpp>
#include <Runtime/Runtime.h>
#include "IR/Module.h"
#include <Python.h>

using namespace std;

namespace eosio {
namespace chain {

class chain_controller;
/**
 * @class python_interface
 *
 * EOS uses the wasm-jit library to evaluate web assembly code. This library relies
 * upon a singlton interface which means there can be only one instance. This interface
 * is designed to wrap that singlton interface and potentially make it thread-local state
 * in the future.
 */
class python_interface {
public:
   struct ModuleState {
      Runtime::ModuleInstance* instance = nullptr;
      IR::Module* module = nullptr;
      int mem_start = 0;
      int mem_end = 1 << 16;
      vector<char> init_memory;
      fc::sha256 code_version;
   };

   static python_interface& get();

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

   python_interface();
};

int wasm_call_function(uint64_t code, uint64_t function, std::vector<uint64_t> args);


uint32_t transactionCreate_();
void transactionRequireScope_(uint32_t handle, string& scope, uint32_t readOnly);
void transactionAddMessage_(uint32_t handle,uint32_t msg_handle);
void transactionSend_(uint32_t handle);
void transactionDrop_(uint32_t handle);
uint32_t messageCreate_(string& code, string& type, string& data);
void messageRequirePermission_(uint32_t handle, string& account,string& permission);
void messageSend_(uint32_t handle);
void messageDrop_(uint32_t handle);


}
} // eos::chain

extern "C" {
   void Py_EnableCodeExecution(int enable, int _only_once);

   int PyTraceMalloc_GetTracebackLimit();
   int PyTraceMalloc_IsOutOffMemory();

   void PyTraceMalloc_SetMaxMallocSize(int nsize);
   int PyTraceMalloc_GetMaxMallocSize();
   void PyTraceMalloc_SetMaxExecutionTime(int nsize);
   int PyTraceMalloc_GetMaxExecutionTime();

   void Py_SetWhiteList(const char** _white_list);
   void Py_EnableImportWhiteList(int enable);
}
