#include <eosio/chain/chain_api.hpp>


namespace eosio { namespace chain {
   static chain_api *s_api = nullptr;
   void register_chain_api(chain_api *api) {
      s_api = api;
   }

   chain_api& get_chain_api() {
      return *s_api;
   }

   }
}
