#include <algorithm>
#include <eosio/chain/apply_context.hpp>
#include <eosio/chain/controller.hpp>
#include <eosio/chain/wasm_interface.hpp>
#include <eosio/chain/micropython_interface.hpp>

#include <eosio/chain/webassembly/wavm.hpp>
#include <eosio/chain/webassembly/binaryen.hpp>
#include <eosio/chain/webassembly/runtime_interface.hpp>
#include <eosio/chain/wasm_eosio_injection.hpp>

#include <eosio/chain/generated_transaction_object.hpp>
#include <eosio/chain/account_object.hpp>

#include <eosio/chain/types.hpp>

#include <eosio/chain_plugin/chain_plugin.hpp>

#include <appbase/application.hpp>

#include "IR/Module.h"
#include "Runtime/Intrinsics.h"
#include "Platform/Platform.h"
#include "WAST/WAST.h"
#include "IR/Validate.h"


#include <fc/io/json.hpp>

#include <fc/crypto/xxhash.h>


//mpeoslib.cpp
extern "C" void eosio_assert(int condition, const char* str);

using namespace fc;
using namespace eosio::chain::webassembly;
using namespace IR;
using namespace Runtime;

namespace eosio { namespace chain {

char code_buffer[128*1024];


class vm_wasm {
//wasm_interface::vm_type vm
public:
   vm_wasm() {
      runtime_wavm = std::make_unique<webassembly::wavm::wavm_runtime>();
      runtime_binaryen = std::make_unique<webassembly::binaryen::binaryen_runtime>();
   }
   static vm_wasm& get() {
      static vm_wasm* instance = 0;
      if (!instance) {
         instance = new vm_wasm();
      }
      return *instance;
   }
   std::vector<uint8_t> parse_initial_memory(const Module& module) {
      std::vector<uint8_t> mem_image;

      for(const DataSegment& data_segment : module.dataSegments) {
         FC_ASSERT(data_segment.baseOffset.type == InitializerExpression::Type::i32_const);
         FC_ASSERT(module.memories.defs.size());
         const U32 base_offset = data_segment.baseOffset.i32;
         const Uptr memory_size = (module.memories.defs[0].type.size.min << IR::numBytesPerPageLog2);
         if(base_offset >= memory_size || base_offset + data_segment.data.size() > memory_size)
            FC_THROW_EXCEPTION(wasm_execution_error, "WASM data segment outside of valid memory range");
         if(base_offset + data_segment.data.size() > mem_image.size())
            mem_image.resize(base_offset + data_segment.data.size(), 0x00);
         memcpy(mem_image.data() + base_offset, data_segment.data.data(), data_segment.data.size());
      }

      return mem_image;
   }

   std::shared_ptr<wasm_instantiated_module_interface>& get_module(const digest_type& code_id, const string& code, wasm_interface::vm_type type = wasm_interface::vm_type::binaryen, bool cache=true ) {
      auto it = instantiation_cache.find(code_id);
      if(!cache || it == instantiation_cache.end() ) {
         IR::Module module;
         try {
            Serialization::MemoryInputStream stream((const U8*)code.c_str(), code.length());
            WASM::serialize(stream, module);
         } catch(Serialization::FatalSerializationException& e) {
            EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
         }

         wasm_injections::wasm_binary_injection injector(module);
         injector.inject();

         std::vector<U8> bytes;
         try {
            Serialization::ArrayOutputStream outstream;
            WASM::serialize(outstream, module);
            bytes = outstream.getBytes();
         } catch(Serialization::FatalSerializationException& e) {
            EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
         }
         if (type == wasm_interface::vm_type::binaryen) {
            it = instantiation_cache.emplace(code_id, runtime_binaryen->instantiate_module((const char*)bytes.data(), bytes.size(), parse_initial_memory(module))).first;
         } else {
            it = instantiation_cache.emplace(code_id, runtime_wavm->instantiate_module((const char*)bytes.data(), bytes.size(), parse_initial_memory(module))).first;
         }
      }
      return it->second;
   }
   std::unique_ptr<webassembly::wavm::wavm_runtime> runtime_wavm;
   std::unique_ptr<wasm_runtime_interface> runtime_binaryen;

   map<digest_type, std::shared_ptr<wasm_instantiated_module_interface>> instantiation_cache;
};

int wasm_test___(string& code, string& func, string& contract, string& action, string& args, map<string, string>& permissions, bool sign, bool rawargs) {
   wlog("+++++++++++++++++wasm_test_:${n}", ("n", contract));
   try {
      //      ilog("Converting argument to binary...");
      auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
      auto rw_api = app().get_plugin<chain_plugin>().get_read_write_api();
      eosio::chain_apis::read_only::abi_json_to_bin_params params;
      if (!rawargs) {
         params = {contract, action, fc::json::from_string(args)};
      } else {
         std::vector<char> v(args.begin(), args.end());
         params = {contract, action, fc::variant(v)};
      }

      vector<chain::permission_level> accountPermissions;
      for (auto it = permissions.begin(); it != permissions.end(); it++) {
         accountPermissions.push_back(chain::permission_level{name(it->first), name(it->second)});
      }

      vector<chain::action> actions;

      if (action.size() == 0) {//evm
            string _args;
            if (args[0] == '0' && args[1] == 'x') {
               _args = string(args.begin()+2, args.end());
            } else {
               _args = args;
            }
            bytes v;
            v.resize(0);
            v.resize(_args.size()/2);
            fc::from_hex(_args, v.data(), v.size());
         actions.emplace_back(accountPermissions, contract, action, v);
      } else {
         auto result = ro_api.abi_json_to_bin(params);
         actions.emplace_back(accountPermissions, contract, action, result.binargs);
      }
#if 0
      vector<uint64_t> _args;
      signed_transaction trx;
      trx.actions = std::forward<decltype(actions)>(actions);

      packed_transaction packed_trx(trx);
      transaction_metadata   mtrx( packed_trx, fc::sha256(), fc::time_point::now());

      chain_controller& mutable_controller = appbase::app().get_plugin<chain_plugin>().chain();

      apply_context ctx(mutable_controller, mutable_controller.get_mutable_database(), actions[0], mtrx);

      auto code_id = fc::sha256::hash(code.c_str(), (uint32_t)code.length());
      vm_wasm::get().get_module(code_id, code)->call(func, _args, ctx);
#endif
   }
   catch (...) {
      return 0;
   }
   return 1;
}

int wasm_test_(string& code, string& func, vector<uint64_t>& args, uint64_t _account, uint64_t _action, vector<char>& data) {
   #if 0
   vector<chain::permission_level> accountPermissions;
   accountPermissions.push_back(chain::permission_level{_account, name("active")});
   action act;
   act.account = _account;
   act.name = _action;
   act.authorization = accountPermissions;
   act.data = data;

   signed_transaction trx;
   packed_transaction packed_trx(trx);

   transaction_metadata   mtrx( packed_trx, fc::sha256(), fc::time_point::now());

   chain_controller& mutable_controller = appbase::app().get_plugin<chain_plugin>().chain();
   apply_context ctx(mutable_controller, mutable_controller.get_mutable_database(), act, mtrx);

   auto code_id = fc::sha256::hash(code.c_str(), (uint32_t)code.length());
   vm_wasm::get().get_module(code_id, code)->call(func, args, ctx);
   #endif
   return 1;
}

uint64_t wasm_call2_(uint64_t account, string& file_name, string& func, vector<uint64_t>& args, vector<char>& result) {

//   chain_controller& mutable_controller = appbase::app().get_plugin<chain_plugin>().chain();
   wlog("${n1}, ${n2}, ${n3}",  ("n1", account)("n2", file_name)("n3", func));

   apply_context& ctx = apply_context::ctx();

   //FIXME key conflict
   uint64_t id = XXH64(file_name.c_str(), file_name.length(), 0);

   int itr = ctx.db_find_i64(account, account, account, id);
   if (itr < 0) {
      return -1;
   }

   int code_size = ctx.db_get_i64(itr, code_buffer, sizeof(code_buffer));
   if (code_size <= 0) {
      return -1;
   }

   eosio_assert( code_buffer[0] == 3, "wrong code type");

   string code(code_buffer+1, (code_size-1));

   auto code_id = fc::sha256::hash(code.c_str(), (uint32_t)code.length());
   uint64_t ret =  vm_wasm::get().get_module(code_id, code)->call(func, args, ctx);
   wlog("call return ${n1}",  ("n1", ret));
   return ret;
}
}}
