#include "../ipc_client/rpc_interface.hpp"

#include <Python.h>
#include <stdio.h>
#include <boost/thread/thread.hpp>

#include <eosio/chain/apply_context.hpp>
#include <fc/exception/exception.hpp>
#include <appbase/application.hpp>


typedef void (*fn_init)();
static fn_init s_init_eos = 0;
static bool client_connected = false;

static fn_rpc_apply rpc_apply = nullptr;
extern "C" void rpc_register_apply_call(fn_rpc_apply fn) {
   rpc_apply = fn;
}

void init_eos() {
   if (s_init_eos) {
      s_init_eos();
   }
}

void set_client_connected() {
   client_connected = true;
}

bool is_client_connected() {
   return client_connected;
}


namespace eosio {
namespace chain {

rpc_interface::rpc_interface() {
}

rpc_interface& rpc_interface::get() {
   static rpc_interface* python = nullptr;
   if (!python) {
      wlog("rpc_interface::init");
      python = new rpc_interface();
   }
   return *python;
}

void rpc_interface::on_setcode(uint64_t _account, bytes& code) {
   assert(rpc_apply != nullptr);
   char *err;
   int len;
   int ret = rpc_apply(_account, N(eosio), N(setcode), &err, &len);
   if (ret != 0) {
      string msg(err, len);
      free(err);
      throw fc::exception(0, "RPC", msg);
   }
}

bool rpc_interface::ready() {
   return rpc_apply != nullptr;
}

void rpc_interface::apply(apply_context& c) {
   assert(rpc_apply != nullptr);
   char *err;
   int len;
   int ret = rpc_apply(c.receiver.value, c.act.account.value, c.act.name.value, &err, &len);
   if (ret != 0) {
     if (ret == 911) {
       //TODO: Fix serious condition
     }
      string msg(err, len);
      wlog(msg);
      free(err);
      throw fc::exception(0, "RPC", msg);
   }
}

}
}
