#include <eosio/chain/controller.hpp>

namespace eosio { namespace chain {
      struct chain_api {
         controller *ctrl;
         controller::config *cfg;
      };

      void register_chain_api(chain_api *api);
      chain_api& get_chain_api();
   }
}
