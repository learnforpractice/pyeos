#include "debug_context.hpp"

//chain/chain_api.cpp
namespace eosio { namespace chain {
   controller& get_chain_controller();
}
}

static controller& chain_controller() { return get_chain_controller(); }


//chain/chain_api.cpp
namespace eosio { namespace chain {
   controller& get_chain_controller();
}
}

debug_context::debug_context() {
   act                  = new action();
   act->account         = N(eosio.code);
   act->authorization   = vector<permission_level>{{N(hello),N(active)}};

   trx_id               = new transaction_id_type();
   signed_trx           = new signed_transaction();
   trx_ctx              = new transaction_context( chain_controller(), *signed_trx, *trx_id );
   ctx                  = new apply_context( chain_controller(), *trx_ctx, *act, 10 );
   ctx->current_context = ctx;
}

debug_context& debug_context::get() {
   static debug_context* instance = nullptr;
   if (!instance) {
      instance = new debug_context();
   }
   return *instance;
}

void debug_context::set_action(action& a) {
   *this->act = a;
}
