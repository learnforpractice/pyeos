/*
 * chain_controller_.cpp
 *
 *  Created on: Oct 26, 2017
 *      Author: newworld
 */
#include <eosio/chain/chain_controller.hpp>
#include <eos/types/generated.hpp>
#include <appbase/application.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eos/chain/python_interface.hpp>
#include <eosio/chain/wasm_interface.hpp>

#include <eosio/chain/account_object.hpp>

#include "blockchain_.hpp"

using namespace eosio;
using namespace eosio::chain;
using namespace eosio::types;
using namespace eosio::chain_apis;
using namespace appbase;

namespace python {
   void pychain_controller::new_apply_context() {
//      apply_context apply_ctx(*this, _db, trx, message, code);
   }

   chain_controller& get_db() { return app().get_plugin<chain_plugin>().chain(); }

   void* new_apply_context_(char* code, void* trx, void* message) {
      if (code == NULL || trx == NULL || message == NULL) {
         return 0;
      }

      eosio::types::transaction *mytrx = (eosio::types::transaction*)trx;
      eosio::types::message *mymsg = (eosio::types::message*)message;

      auto& ctrl = get_db();

      apply_context* apply_ctx = new apply_context(ctrl, *(database*)(&ctrl.get_database()),
            *(chain::transaction*)mytrx, *(chain::message*)message, name(code));

      return apply_ctx;
   }

   int release_apply_context_(void* apply_ctx) {
      if (apply_ctx == NULL) {
         return -1;
      }
      delete (apply_context*)apply_ctx;
      return 0;
   }

   void set_current_context_(void* context_ptr) {
      apply_context *ctx;
      if (context_ptr == NULL) {
         return;
      }
      ctx = (apply_context*)context_ptr;
      auto code = ctx->msg.code.to_string();
      python_interface::get().set_current_context(*ctx);
   }

   void apply_message_(void* context_ptr)
   {
      apply_context *ctx;
      if (context_ptr == NULL) {
         return;
      }
      ctx = (apply_context*)context_ptr;

      try {

       const auto& recipient = get_db().get_database().get<account_object,by_name>(ctx->code);

       if (recipient.code.size() > 0) {
          //idump((context.code)(context.msg.type));
          const uint32_t execution_time = 1000*200; //200 ms
          if (recipient.vm_type == 0){
             wasm_interface::get().apply(*ctx, execution_time, false);
          } else if (recipient.vm_type == 1) {
             ilog("python_interface::get().apply(*ctx);");
              python_interface::get().apply(*ctx);
          }
       }

   } FC_CAPTURE_AND_RETHROW((ctx->msg)) }
}


