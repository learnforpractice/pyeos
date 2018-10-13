#pragma once

#include <eosio/chain/transaction.hpp>
#include <eosio/chain/transaction_context.hpp>
#include <eosio/chain/apply_context.hpp>
#include <eosio/chain/controller.hpp>

using namespace eosio::chain;

class debug_context {
public:
   debug_context();
   static debug_context& get();
   void set_action(action& a);
   action* act;
   signed_transaction* signed_trx;
   transaction_id_type* trx_id;
   transaction_context* trx_ctx;
   apply_context*  ctx;
};
