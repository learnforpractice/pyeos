#pragma once
#include <eosio/chain/types.hpp>

namespace eosio { namespace chain {

   class apply_context;
   class wasm_runtime_interface;

   struct wasm_exit {
      int32_t code = 0;
   };

   namespace webassembly { namespace common {
      class intrinsics_accessor;
   } }

   /**
    * @class wasm_interface
    *
    */
   class wasm_interface {
      public:
         enum class vm_type {
            wavm,
            binaryen,
         };

         wasm_interface(vm_type vm);
         ~wasm_interface();

         //validates code -- does a WASM validation pass and checks the wasm against EOSIO specific constraints
         static void validate(const bytes& code);

         void call( const digest_type& code_id, const shared_string& code, string& func, vector<uint64_t>& args, apply_context& context );

         //Calls apply or error on a given code
         void apply(uint64_t receiver, uint64_t account, uint64_t act);
         bool init();
      private:
         unique_ptr<struct wasm_interface_impl> my;
         friend class eosio::chain::webassembly::common::intrinsics_accessor;
   };

} } // eosio::chain

namespace eosio{ namespace chain {
   std::istream& operator>>(std::istream& in, wasm_interface::vm_type& runtime);
}}

FC_REFLECT_ENUM( eosio::chain::wasm_interface::vm_type, (wavm)(binaryen) )
