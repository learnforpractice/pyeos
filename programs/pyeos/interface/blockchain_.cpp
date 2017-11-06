/*
 * chain_controller_.cpp
 *
 *  Created on: Oct 26, 2017
 *      Author: newworld
 */
#include <eos/chain/chain_controller.hpp>
#include <eos/chain/message_handling_contexts.hpp>
#include <eos/types/generated.hpp>
#include <appbase/application.hpp>
#include <eos/chain_plugin/chain_plugin.hpp>
#include <eos/chain/python_interface.hpp>

#include "blockchain_.hpp"

using namespace eos;
using namespace eos::chain;
using namespace eos::types;
using namespace eos::chain_apis;
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

      eos::types::Transaction *mytrx = (eos::types::Transaction*)trx;
      eos::types::Message *mymsg = (eos::types::Message*)message;

      auto& ctrl = get_db();

      apply_context* apply_ctx = new apply_context(ctrl, *(database*)(&ctrl.get_database()),
            *(chain::Transaction*)mytrx, *(chain::Message*)message, Name(code));

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
      auto code = ctx->msg.code.toString();
      python_interface::get().set_current_context(*ctx);
   }

}


