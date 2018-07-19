#pragma once

#include "wasm_interface.hpp"

#ifdef _WAVM
#include <eosio/chain/webassembly/wavm.hpp>
#endif

#ifdef _BINARYEN
#include <eosio/chain/webassembly/binaryen.hpp>
#endif

#include <eosio/chain/webassembly/runtime_interface.hpp>
#include <eosio/chain/wasm_eosio_injection.hpp>
#include <eosio/chain/transaction_context.hpp>
#include <eosio/chain/wast_to_wasm.hpp>
#include <boost/thread/thread.hpp>

#include <fc/scoped_exit.hpp>
#include <fc/io/fstream.hpp>

#include "IR/Module.h"
#include "Runtime/Intrinsics.h"
#include "Platform/Platform.h"
#include "WAST/WAST.h"
#include "IR/Validate.h"

#include <mutex>

#include <dlfcn.h>

using namespace fc;
using namespace eosio::chain::webassembly;
using namespace IR;
using namespace Runtime;

void resume_billing_timer();
void pause_billing_timer();
const char* get_code( uint64_t receiver, size_t* size );
int get_code_id( uint64_t account, char* code_id, size_t size );
bool vm_is_account(uint64_t account);

int db_api_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int32_t db_api_get_i64_ex( int iterator, uint64_t* primary, char* buffer, size_t buffer_size );
const char* db_api_get_i64_exex( int itr, size_t* buffer_size );


namespace eosio { namespace chain {
#include <eosiolib_native/vm_api.h>

   struct wasm_interface_impl {
      wasm_interface_impl(wasm_interface::vm_type vm) {
#ifdef _WAVM
         runtime_interface = std::make_unique<webassembly::wavm::wavm_runtime>();
#endif
#ifdef _BINARYEN
         runtime_interface = std::make_unique<webassembly::binaryen::binaryen_runtime>();
#endif
         //init_native_contract();
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

      std::unique_ptr<wasm_instantiated_module_interface>& get_instantiated_module( const uint64_t& receiver, bool preload = false )
      {
         {
            size_t size = 0;
            const char* code;
            char code_id[8*4];

            std::lock_guard<std::mutex> lock(m);

            code = get_code( receiver, &size );
            get_code_id(receiver, code_id, sizeof(code_id));
            if (size <= 0) {
               EOS_ASSERT(false, asset_type_exception, "code size should not be zero");
            }

            auto it = instantiation_cache.find(receiver);
            if (it != instantiation_cache.end()) {
               if (0 == memcmp(code_id, it->second->code_id, sizeof(code_id))) {
                  return it->second;
               }
            }
         }


#ifdef _BINARYEN
         auto timer_pause = fc::make_scoped_exit([&](){
            if (!preload) {
               resume_billing_timer();
            }
         });
         if (!preload) {
            pause_billing_timer();
         }
         return load_module(receiver, code, size);
#endif

#ifdef _WAVM
         elog("update code in wavm ${n}", ("n", name(receiver)));
         if (preload) {
            return load_module(receiver, code, size);
         }

         boost::thread t(boost::bind(&wasm_interface_impl::load_module_async, this, receiver, code, size));

         return instantiation_cache.end()->second;
#endif
      }


       void load_module_async(uint64_t receiver, const char* code, size_t size) {
          load_module(receiver, code, size);
          //send a transaction to indicate that module is loaded by BP.
       }

      std::unique_ptr<wasm_instantiated_module_interface>& load_module(uint64_t receiver, const char* code, size_t size) {
         IR::Module module;
         try {
            Serialization::MemoryInputStream stream((const U8*)code, size);
            WASM::serialize(stream, module);
            module.userSections.clear();
         } catch(const Serialization::FatalSerializationException& e) {
            EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
         } catch(const IR::ValidationException& e) {
            EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
         }

         wasm_injections::wasm_binary_injection injector(module);
         injector.inject();

         std::vector<U8> bytes;
         try {
            Serialization::ArrayOutputStream outstream;
            WASM::serialize(outstream, module);
            bytes = outstream.getBytes();
         } catch(const Serialization::FatalSerializationException& e) {
            EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
         } catch(const IR::ValidationException& e) {
            EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
         }
         {
            std::lock_guard<std::mutex> lock(m);
            instantiation_cache[receiver] = runtime_interface->instantiate_module((const char*)bytes.data(), bytes.size(), parse_initial_memory(module));
            return instantiation_cache.find(receiver)->second;
         }
      }

      int unload_module(uint64_t account) {
         std::lock_guard<std::mutex> lock(m);
         auto it = instantiation_cache.find(account);
         if (it != instantiation_cache.end()) {
            instantiation_cache.erase(it);
            return 1;
         }
         return 0;
      }

      std::unique_ptr<wasm_instantiated_module_interface>& get_instantiated_module()
      {
         const uint64_t receiver = 0;
         {
            std::lock_guard<std::mutex> lock(m);
            auto it = instantiation_cache.find(receiver);
            if (it != instantiation_cache.end()) {
               return it->second;
            }
         }

         string wast;
         fc::read_file_contents("../../programs/pyeos/contracts/lab/lab.wast", wast);
         std::vector<uint8_t> wasm = wast_to_wasm(wast);

         IR::Module module;
         try {
            Serialization::MemoryInputStream stream((const U8*)wasm.data(), wasm.size());
            WASM::serialize(stream, module);
            module.userSections.clear();
         } catch(const Serialization::FatalSerializationException& e) {
            EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
         } catch(const IR::ValidationException& e) {
            EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
         }

         wasm_injections::wasm_binary_injection injector(module);
         injector.inject();

         std::vector<U8> bytes;
         try {
            Serialization::ArrayOutputStream outstream;
            WASM::serialize(outstream, module);
            bytes = outstream.getBytes();
         } catch(const Serialization::FatalSerializationException& e) {
            EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
         } catch(const IR::ValidationException& e) {
            EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
         }

         {
            std::lock_guard<std::mutex> lock(m);
            instantiation_cache[receiver] = runtime_interface->instantiate_module((const char*)bytes.data(), bytes.size(), parse_initial_memory(module));
            return instantiation_cache.find(receiver)->second;
         }
      }
      std::mutex m;
      std::unique_ptr<wasm_runtime_interface> runtime_interface;
      map<uint64_t, std::unique_ptr<wasm_instantiated_module_interface>> instantiation_cache;
   };

#ifdef _WAVM
#define _REGISTER_INTRINSIC_EXPLICIT(CLS, MOD, METHOD, WASM_SIG, NAME, SIG)\
   _REGISTER_WAVM_INTRINSIC(CLS, MOD, METHOD, WASM_SIG, NAME, SIG)
#endif

#ifdef _BINARYEN
#define _REGISTER_INTRINSIC_EXPLICIT(CLS, MOD, METHOD, WASM_SIG, NAME, SIG)\
   _REGISTER_BINARYEN_INTRINSIC(CLS, MOD, METHOD, WASM_SIG, NAME, SIG)
#endif

#define _REGISTER_INTRINSIC4(CLS, MOD, METHOD, WASM_SIG, NAME, SIG)\
   _REGISTER_INTRINSIC_EXPLICIT(CLS, MOD, METHOD, WASM_SIG, NAME, SIG )

#define _REGISTER_INTRINSIC3(CLS, MOD, METHOD, WASM_SIG, NAME)\
   _REGISTER_INTRINSIC_EXPLICIT(CLS, MOD, METHOD, WASM_SIG, NAME, decltype(&CLS::METHOD) )

#define _REGISTER_INTRINSIC2(CLS, MOD, METHOD, WASM_SIG)\
   _REGISTER_INTRINSIC_EXPLICIT(CLS, MOD, METHOD, WASM_SIG, BOOST_PP_STRINGIZE(METHOD), decltype(&CLS::METHOD) )

#define _REGISTER_INTRINSIC1(CLS, MOD, METHOD)\
   static_assert(false, "Cannot register " BOOST_PP_STRINGIZE(CLS) ":" BOOST_PP_STRINGIZE(METHOD) " without a signature");

#define _REGISTER_INTRINSIC0(CLS, MOD, METHOD)\
   static_assert(false, "Cannot register " BOOST_PP_STRINGIZE(CLS) ":<unknown> without a method name and signature");

#define _UNWRAP_SEQ(...) __VA_ARGS__

#define _EXPAND_ARGS(CLS, MOD, INFO)\
   ( CLS, MOD, _UNWRAP_SEQ INFO )

#define _REGISTER_INTRINSIC(R, CLS, INFO)\
   BOOST_PP_CAT(BOOST_PP_OVERLOAD(_REGISTER_INTRINSIC, _UNWRAP_SEQ INFO) _EXPAND_ARGS(CLS, "env", INFO), BOOST_PP_EMPTY())

#define REGISTER_INTRINSICS(CLS, MEMBERS)\
   BOOST_PP_SEQ_FOR_EACH(_REGISTER_INTRINSIC, CLS, _WRAPPED_SEQ(MEMBERS))

#define _REGISTER_INJECTED_INTRINSIC(R, CLS, INFO)\
   BOOST_PP_CAT(BOOST_PP_OVERLOAD(_REGISTER_INTRINSIC, _UNWRAP_SEQ INFO) _EXPAND_ARGS(CLS, EOSIO_INJECTED_MODULE_NAME, INFO), BOOST_PP_EMPTY())

#define REGISTER_INJECTED_INTRINSICS(CLS, MEMBERS)\
   BOOST_PP_SEQ_FOR_EACH(_REGISTER_INJECTED_INTRINSIC, CLS, _WRAPPED_SEQ(MEMBERS))

} } // eosio::chain
