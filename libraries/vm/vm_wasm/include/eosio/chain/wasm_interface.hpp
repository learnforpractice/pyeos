#pragma once
#include <eosio/chain/types.hpp>
#include "Runtime/Linker.h"
#include "Runtime/Runtime.h"

using namespace std;

namespace eosio { namespace chain {

   class wasm_runtime_interface;

   struct wasm_exit {
      int32_t code = 0;
   };

   namespace webassembly { namespace common {
      class intrinsics_accessor;
      struct root_resolver : Runtime::Resolver {
         //when validating is true; only allow "env" imports. Otherwise allow any imports. This resolver is used
         //in two cases: once by the generic validating code where we only want "env" to pass; and then second in the
         //wavm runtime where we need to allow linkage to injected functions
         root_resolver(bool validating = false) : validating(validating) {}
         bool validating;

         bool resolve(const string& mod_name,
                      const string& export_name,
                      IR::ObjectType type,
                      Runtime::ObjectInstance*& out) override {
      try {
         //protect access to "private" injected functions; so for now just simply allow "env" since injected functions
         //  are in a different module
         if(validating && mod_name != "env")
            FC_ASSERT( !"importing from module that is not 'env'", "${module}.${export}", ("module",mod_name)("export",export_name) );

         // Try to resolve an intrinsic first.
         if(Runtime::IntrinsicResolver::singleton.resolve(mod_name,export_name,type, out)) {
            return true;
         }

         FC_ASSERT( !"unresolvable", "${module}.${export}", ("module",mod_name)("export",export_name) );
         return false;
      } FC_CAPTURE_AND_RETHROW( (mod_name)(export_name) ) }
      };
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
         static wasm_interface& get();
         ~wasm_interface();

         int setcode(uint64_t account);
         uint64_t call(string& func, vector<uint64_t>& args );
         //Calls apply or error on a given code
         int apply(uint64_t receiver, uint64_t account, uint64_t act);
         bool init();
         int preload(uint64_t account);
         int unload(uint64_t account);

      private:
         wasm_interface(vm_type vm);
         unique_ptr<struct wasm_interface_impl> my;
         friend class eosio::chain::webassembly::common::intrinsics_accessor;
   };

} } // eosio::chain

namespace eosio{ namespace chain {
   std::istream& operator>>(std::istream& in, wasm_interface::vm_type& runtime);
}}
