#include <appbase/platform.hpp>
#include <memory>
#include <map>

#include <eosiolib_native/vm_api.h>

using namespace std;

#if defined(__APPLE__) && defined(__MACH__)
   #define NATIVE_PLATFORM 1
#elif defined(__linux__)
   #define NATIVE_PLATFORM 2
#elif defined(_WIN64)
   #define NATIVE_PLATFORM 3
#else
   #error Not Supported Platform
#endif


namespace eosio { namespace chain {


   struct native_code_cache {
         uint32_t version;
         void *handle;
         fn_apply apply;
   };

   class native_interface {
   private:
      native_interface();
   public:
      static native_interface & get();
      void init_native_contract();

      fn_apply load_native_contract(uint64_t _account);
      fn_apply load_native_contract_default(uint64_t _account);
      int apply(uint64_t receiver, uint64_t account, uint64_t act);
   private:
      fn_apply _bios_apply;
      fn_apply _msig_apply;
      fn_apply _token_apply;
      fn_apply _eosio_apply;
      fn_apply _exchange_apply;

      map<uint64_t, std::unique_ptr<native_code_cache>> native_cache;
   };

} } // eosio::chain

