#include <eosio/chain/wasm_interface.hpp>
#include <eosio/chain/apply_context.hpp>
#include <eosio/chain/controller.hpp>
#include <eosio/chain/transaction_context.hpp>
#include <eosio/chain/producer_schedule.hpp>
#include <eosio/chain/exceptions.hpp>
#include <boost/core/ignore_unused.hpp>
#include <eosio/chain/authorization_manager.hpp>
#include <eosio/chain/resource_limits.hpp>
/*
#include <eosio/chain/wasm_interface_private.hpp>
#include <eosio/chain/wasm_eosio_validation.hpp>
#include <eosio/chain/wasm_eosio_injection.hpp>
*/
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/account_object.hpp>
//#include <eosio/chain/symbol.hpp>

#include <fc/exception/exception.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/sha1.hpp>
#include <fc/io/raw.hpp>

#include <softfloat.hpp>
//#include <compiler_builtins.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <fstream>

#include <fc/crypto/xxhash.h>
#include <dlfcn.h>

#include <eosio/chain/db_api.h>
#include <vm_manager.hpp>
//#include <appbase/application.hpp>
#include "ipc_client.hpp"

#include <appbase/application.hpp>

namespace eosio {
namespace chain {

static inline apply_context& ctx() {
   return apply_context::ctx();
}

#include <eosiolib_native/vm_api.h>

#include "action.cpp"
#include "chain.cpp"
#include "system.cpp"
#include "crypto.cpp"
#include "db.cpp"
#include "privileged.cpp"
#include "transaction.cpp"
#include "print.cpp"
#include "permission.cpp"

uint64_t get_action_account() {
   return db_api::get().get_action_object().account.value;
}

extern "C" uint64_t string_to_uint64_(const char* str) {
   try {
      return name(str).value;
   } catch (...) {
   }
   return 0;
}

void set_code(uint64_t user_account, int vm_type, const char* code, int code_size) {
   FC_ASSERT(false, "fixme");
}

const char* get_code( uint64_t receiver, size_t* size ) {
   const shared_string& src = db_api::get().get_code(receiver);
   *size = src.size();
   return src.data();
}

int get_code_id( uint64_t account, char* code_id, size_t size ) {
   digest_type id = db_api::get().get_code_id(account);
   if (size != sizeof(id)) {
      return 0;
   }
   memcpy(code_id, id._hash, size);
   return 1;
}

int has_option(const char* _option) {
   return appbase::app().has_option(_option);
}

int get_option(const char* option, char *result, int size) {
   return appbase::app().get_option(option, result, size);
}

int32_t uint64_to_string_(uint64_t n, char* out, int size) {
   if (out == NULL || size == 0) {
      return 0;
   }

   string s = name(n).to_string();
   if (s.length() < size) {
      size = s.length();
   }
   memcpy(out, s.c_str(), size);
   return size;
}

void resume_billing_timer() {
//   ctx().trx_context.resume_billing_timer();
}

void pause_billing_timer() {
//   ctx().trx_context.pause_billing_timer();
}

int run_mode() // 0 for server, 1 for client
{
   return 1;
}

static struct vm_api _vm_api = {
};

void vm_manager_init(int vm_type) {
   vm_register_api(&_vm_api);
   vm_manager::get().set_vm_api(&_vm_api);
   vm_manager::get().load_vm(vm_type);
//eosiolib_native/eosiolib.cpp
}

std::istream& operator>>(std::istream& in, wasm_interface::vm_type& runtime) {
   std::string s;
   in >> s;
   if (s == "wavm")
      runtime = eosio::chain::wasm_interface::vm_type::wavm;
   else if (s == "wabt")
      runtime = eosio::chain::wasm_interface::vm_type::wabt;
   else
      in.setstate(std::ios_base::failbit);
   return in;
}


}}
