#pragma once
#include <vector>
#include <memory>
#include <stdint.h>

namespace eosio { namespace chain {

class apply_context;

class wasm_instantiated_module_interface {
   public:
      virtual void apply(apply_context& context) = 0;
      virtual uint64_t call(const std::string &entry_point, const std::vector <uint64_t> & _args, apply_context &context) = 0;


      virtual ~wasm_instantiated_module_interface();
};

class wasm_runtime_interface {
   public:
      virtual std::unique_ptr<wasm_instantiated_module_interface> instantiate_module(const char* code_bytes, size_t code_size, std::vector<uint8_t> initial_memory) = 0;

      virtual ~wasm_runtime_interface();
};

}}
