#include <eosio/chain/chain_api.hpp>
#include <eosio/chain/chain_api.h>


static chain_api *s_api = nullptr;
void register_chain_api(chain_api *api) {
   s_api = api;
}

chain_api& get_chain_api() {
//   EOS_ASSERT(s_api, chain_exception, "chain api not initialized!");
   return *s_api;
}

string get_state_dir() {
   return get_chain_api().state_dir();
}
