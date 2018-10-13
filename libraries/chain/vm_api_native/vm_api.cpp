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
#include <eosio/chain/symbol.hpp>

#include <fc/exception/exception.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/sha1.hpp>
#include <fc/io/raw.hpp>

#include <softfloat.hpp>
#include <compiler_builtins.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <fstream>

#include <fc/crypto/xxhash.h>
#include <dlfcn.h>

#include <vm_manager.hpp>
#include <appbase/application.hpp>

#include <eosio/chain/db_api.h>
#include <eosio/chain/options.hpp>

#include <eosiolib_native/vm_api.hpp>

using namespace eosio::chain;
#include <appbase/platform.hpp>
#include <dlfcn.h>

extern "C" void vm_api_init();

class init_vm_api {
public:
   init_vm_api() {
      vm_api_init();
   }
};

static init_vm_api _init_vm_api;


//exceptions.cpp
void fc_throw_exception(int type, const char* fmt, ...);

static vector<char> s_args;
static vector<char> s_results;
static int s_call_status = 0;

//native/native.cpp
int transfer_inline(uint64_t to, int64_t amount, uint64_t symbol);
int transfer(uint64_t from, uint64_t to, int64_t amount, uint64_t symbol);

void db_store_i64_in_account( uint64_t code, uint64_t scope, uint64_t table_id, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   apply_context::ctx().db_store_i64( code, scope, table_id, payer, id, buffer, buffer_size );
}

void db_update_i64_without_check_code( int iterator, uint64_t payer, const char* buffer, size_t buffer_size ) {
   apply_context::ctx().db_update_i64( iterator, payer, buffer, buffer_size, false );
}

void db_remove_i64_without_check_code( int iterator ) {
   apply_context::ctx().db_remove_i64( iterator, false );
}


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
   return ctx().act.account.value;
}

extern "C" uint64_t string_to_uint64_(const char* str) {
   try {
      return name(str).value;
   } catch (...) {
   }
   return 0;
}

int get_balance(uint64_t _account, uint64_t _symbol, int64_t* amount) {
   struct {
      int64_t amount;
      uint64_t symbol;
   }balance;
   auto itr = db_find_i64(N(eosio.token), _account, N(accounts), _symbol>>8);
   if (itr<0) {
      return 0;
   }
   db_get_i64(itr, &balance, sizeof(balance));
   *amount = balance.amount;
   return 1;
}

void set_code(uint64_t user_account, int vm_type, const char* code, int code_size) {
   FC_ASSERT(code != NULL && code_size != 0);

   const auto& account = ctx().db.get<account_object,by_name>(user_account);

   auto code_id = fc::sha256::hash( code, (uint32_t)code_size );

   int64_t old_size  = (int64_t)account.code.size() * config::setcode_ram_bytes_multiplier;
   int64_t new_size  = code_size * config::setcode_ram_bytes_multiplier;

   ctx().db.modify( account, [&]( auto& a ) {
      a.vm_type = vm_type;
      a.last_code_update = ctx().control.pending_block_time();
      a.code_version = code_id;
      a.code.resize( code_size );
      if( code_size > 0 )
         memcpy( a.code.data(), code, code_size );
   });

   if (new_size != old_size) {
      ctx().add_ram_usage( user_account, new_size - old_size );
   }

}

bool check_code_auth(uint64_t account, uint64_t code_account, uint64_t code_name) {
   //TODO check authorization of code
   if (account == code_account) {
      return true;
   }
   auto itr = db_find_i64(N(eosio.code), code_account, account, code_name);
   if (itr >= 0) {
      return true;
   }
   return false;
}

int set_code_ext(uint64_t account, int vm_type, uint64_t code_name, const char* src_code, size_t code_size) {
   int result_size = 0;
   const char* compiled_code;

   get_vm_api()->require_auth(account);

   if (vm_type == VM_TYPE_PY) {
      compiled_code = get_vm_api()->vm_cpython_compile(name(account).to_string().c_str(), src_code, code_size, &result_size);
   }

   uint64_t payer = account;
   uint64_t scope = N(eosio.code);
   uint64_t table_id = account;
   auto itr = db_find_i64(N(eosio.code), scope, table_id, code_name);
   if (itr >= 0) {
      if (code_size == 0 || src_code == nullptr) {
         db_remove_i64(itr);
      } else {
         db_update_i64(itr, payer, compiled_code, result_size);
      }
   } else {
      db_store_i64(scope, table_id, payer, code_name, compiled_code, result_size);
   }
   return 1;
}

const char* load_code_ext(uint64_t code_account, uint64_t code_name, size_t* code_size) {
   uint64_t code = N(eosio.code);
   uint64_t scope = code;
   uint64_t table_id = code_account;
   uint64_t id = code_name;

   uint64_t receiver = get_vm_api()->current_receiver();
   if (!get_vm_api()->check_code_auth(receiver, code_account, code_name)) {
      *code_size = 0;
      return nullptr;
   }

#if 0
   int itr = db_find_i64(code, scope, table_id, id);
   if (itr >= 0) {
      return db_get_i64_exex(itr, code_size);
   } else {
      return NULL;
   }
#else
   int itr = db_api_find_i64(code, scope, table_id, id);
   if (itr <0) {
      *code_size = 0;
      return nullptr;
   }
   return db_api_get_i64_exex( itr, code_size );
#endif
}

bool is_code_activated( uint64_t account ) {
   try {
      const auto& account_obj = ctx().db.get<account_object,by_name>(account);
      return account_obj.code_activated;
   } catch (...) {

   }
   return false;
}

const char* get_code( uint64_t receiver, size_t* size ) {
   try {
      if (!ctx().is_account(receiver)) {
         *size = 0;
         return nullptr;
      }

      auto& a = ctx().control.get_account(receiver);
      *size = a.code.size();
      return a.code.data();
   } catch (...) {

   }

   if (!db_api::get().is_account(receiver)) {
      *size = 0;
      return nullptr;
   }

   try {
      const shared_string& src = db_api::get().get_code(receiver);
      *size = src.size();
      return src.data();
   } catch (...) {
      *size = 0;
      return nullptr;
   }
}

int get_code_id( uint64_t account, char* code_id, size_t size ) {
   if (!db_api::get().is_account(account)) {
      return 0;
   }
   try {
      digest_type id = db_api::get().get_code_id(account);
      if (size != sizeof(id)) {
         return 0;
      }
      memcpy(code_id, id._hash, size);
      return 1;
   } catch (...) {
      return 0;
   }
}

int get_code_type( uint64_t account) {
   if (!is_account(account)) {
      return -1;
   }
   const auto& a = ctx().db.get<account_object,by_name>(account);
   return a.vm_type;
}

int has_option(const char* _option) {
   try {
      return appbase::app().has_option(_option);
   } catch (...) {
      return 0;
   }
}

int get_option(const char* option, char *result, int size) {
   try {
      return appbase::app().get_option(option, result, size);
   } catch (...) {
      return 0;
   }
}

int app_init_finished() {
   return appbase::app().app_init_finished();
}

int32_t uint64_to_string_(uint64_t n, char* out, int size) {
   if (out == NULL || size == 0) {
      return name(n).to_string().size();
   }

   string s = name(n).to_string();
   if (s.length() < size) {
      size = s.length();
   }
   memcpy(out, s.c_str(), size);
   return size;
}

void resume_billing_timer() {
   ctx().trx_context.resume_billing_timer();
}

void pause_billing_timer() {
   ctx().trx_context.pause_billing_timer();
}

int run_mode() // 0 for server, 1 for client
{
   return 0;
}

uint64_t wasm_call(const char*func, uint64_t* args , int argc) {
   vector<uint64_t> _args;
   for (int i=0;i<argc;i++) {
      _args.push_back(args[i]);
   }
   return vm_manager::get().wasm_call(string(func), _args);
}

int call_set_args(const char* args , int len) {
   if (args == NULL || len <=0) {
      return 0;
   }
   s_args.resize(len);
   memcpy(s_args.data(), args, len);
   return 1;
}


int call_get_args(char* args , int len) {
   if (args == NULL || len <=0) {
      return s_args.size();
   }

   int copy_len = std::min(s_args.size(), (size_t)len);
   memcpy(args, s_args.data(), copy_len);
   return copy_len;
}

int call(uint64_t account, uint64_t func) {
   int ret = 0;
   s_results.resize(0);
   s_call_status = 1;
   try {
      ret = vm_manager::get().call(account, func);
   } catch (...) {
      s_call_status = 0;
      throw;
   }
   s_call_status = 0;
   return ret;
}

int get_call_status() {
   return s_call_status;
}

int call_set_results(const char* result , int len) {
   if (result == NULL || len <=0) {
      return 0;
   }
   s_results.resize(len);
   memcpy(s_results.data(), result, len);
   return 1;
}

int call_get_results(char* result , int len) {
   if (result == NULL || len <= 0) {
      return s_results.size();
   }
   int copy_len = std::min(s_results.size(), (size_t)len);
   memcpy(result, s_results.data(), copy_len);
   return copy_len;
}

bool is_replay() {
   return ctx().trx_context.deadline == fc::time_point::maximum();
}


void update_db_usage( uint64_t payer, int64_t delta ) {
   ctx().update_db_usage(payer, delta);
}

bool verify_account_ram_usage( uint64_t account ) {
   return ctx().control.get_mutable_resource_limits_manager().verify_account_ram_usage_ex(account);
}

static vector<char> print_buffer;

void log_(int level, int line, const char *file, const char *func, const char *fmt, ...) {
//void log_(const char *output, int level, int line, const char *file, const char *func) {
   char output[256];
   va_list args;
   va_start(args, fmt);
   int len = vsnprintf(output, sizeof output, fmt, args);
   va_end(args);

   for (int i=0;i<len;i++) {
      if (output[i] == '\n') {
         string s(print_buffer.data(), print_buffer.size());

         FC_MULTILINE_MACRO_BEGIN
          if( (fc::logger::get(DEFAULT_LOGGER)).is_enabled( (fc::log_level::values)level ) )
             (fc::logger::get(DEFAULT_LOGGER)).log( fc::log_message( fc::log_context((fc::log_level::values)level, file, line, func ), s.c_str() ));
         FC_MULTILINE_MACRO_END;

         print_buffer.clear();

         continue;
      }
      print_buffer.push_back(output[i]);
   }
}

bool is_debug_mode_() {
   return appbase::app().debug_mode();
}

bool is_unittest_mode() {
   return false;
}

void set_debug_mode(bool b) {
   appbase::app().set_debug_mode(b);
}

bool is_producing_block() {
   return ctx().control.is_producing_block();
}

int get_wasm_runtime_type() {
   return options::get().get_wasm_runtime_type();
}

int is_contracts_console_enabled() {
   return options::get().is_contracts_console_enabled();
}

static struct vm_api _vm_api = {
};

static struct vm_api_cpp _vm_api_cpp = {
};

extern "C" void vm_manager_init() {
   //action.cpp
   vm_register_api(&_vm_api);
   vm_manager::get().init(&_vm_api);

   s_args.reserve(256);
   s_results.reserve(256);
}

static int vm_apply(int type, uint64_t receiver, uint64_t account, uint64_t act) {
   try {
      return vm_manager::get().apply( type, receiver, account, act);
   } FC_LOG_AND_DROP();
   return 0;
}

int64_t get_current_exception(string& what) {
   if (!fc::exception::current_exception) {
      return 0;
   }
   what = fc::exception::current_exception->to_detail_string();
   return fc::exception::current_exception->code();
}


static bool s_init = false;
extern "C" void vm_api_init() {
   if (!s_init){
      s_init = true;
      _vm_api.vm_apply = vm_apply;
      _vm_api.read_action_data = read_action_data;
      _vm_api.action_data_size = action_data_size;
      _vm_api.require_recipient = require_recipient;
      _vm_api.require_auth = require_auth;
      _vm_api.require_auth2 = require_auth2;
      _vm_api.has_auth = has_auth;
      _vm_api.is_account = is_account;
      _vm_api.is_code_activated = is_code_activated;
      _vm_api.is_replay = is_replay;

      _vm_api.send_inline = send_inline;
      _vm_api.send_context_free_inline = send_context_free_inline;
      _vm_api.publication_time = publication_time;
      _vm_api.current_receiver = current_receiver;
      _vm_api.get_active_producers = get_active_producers;


      _vm_api.get_balance = get_balance;
      _vm_api.transfer_inline = transfer_inline;
      _vm_api.transfer = transfer;

      _vm_api.assert_sha256 = assert_sha256;
      _vm_api.assert_sha1 = assert_sha1;
      _vm_api.assert_sha512 = assert_sha512;
      _vm_api.assert_ripemd160 = assert_ripemd160;
      _vm_api.assert_recover_key = assert_recover_key;
      _vm_api.sha256 = sha256;
      _vm_api.sha1 = sha1;
      _vm_api.sha512 = sha512;
      _vm_api.ripemd160 = ripemd160;
      _vm_api.recover_key = recover_key;
      _vm_api.sha3 = nullptr;

      _vm_api.get_table_item_count = get_table_item_count;

      _vm_api.db_store_i64 = db_store_i64;
      _vm_api.db_store_i64_ex = db_store_i64_ex;
      _vm_api.db_update_i64 = db_update_i64;
      _vm_api.db_remove_i64 = db_remove_i64;
      _vm_api.db_update_i64_ex = db_update_i64_ex;

      _vm_api.db_remove_i64_ex = db_remove_i64_ex;
      _vm_api.db_get_i64 = db_get_i64;
      _vm_api.db_get_i64_ex = db_get_i64_ex;
      _vm_api.db_get_i64_exex = db_get_i64_exex;
      _vm_api.db_next_i64 = db_next_i64;

      _vm_api.db_previous_i64 = db_previous_i64;
      _vm_api.db_find_i64 = db_find_i64;
      _vm_api.db_lowerbound_i64 = db_lowerbound_i64;
      _vm_api.db_upperbound_i64 = db_upperbound_i64;
      _vm_api.db_end_i64 = db_end_i64;


      _vm_api.db_store_i256 = db_store_i256;
      _vm_api.db_update_i256 = db_update_i256;
      _vm_api.db_remove_i256 = db_remove_i256;
      _vm_api.db_get_i256 = db_get_i256;
      _vm_api.db_find_i256 = db_find_i256;




      _vm_api.db_idx64_store = db_idx64_store;
      _vm_api.db_idx64_update = db_idx64_update;

      _vm_api.db_idx64_remove = db_idx64_remove;
      _vm_api.db_idx64_next = db_idx64_next;
      _vm_api.db_idx64_previous = db_idx64_previous;
      _vm_api.db_idx64_find_primary = db_idx64_find_primary;
      _vm_api.db_idx64_find_secondary = db_idx64_find_secondary;
      _vm_api.db_idx64_lowerbound = db_idx64_lowerbound;
      _vm_api.db_idx64_upperbound = db_idx64_upperbound;
      _vm_api.db_idx64_end = db_idx64_end;
      _vm_api.db_idx128_store = db_idx128_store;

      _vm_api.db_idx128_update = db_idx128_update;
      _vm_api.db_idx128_remove = db_idx128_remove;
      _vm_api.db_idx128_next = db_idx128_next;
      _vm_api.db_idx128_previous = db_idx128_previous;
      _vm_api.db_idx128_find_primary = db_idx128_find_primary;
      _vm_api.db_idx128_find_secondary = db_idx128_find_secondary;
      _vm_api.db_idx128_lowerbound = db_idx128_lowerbound;
      _vm_api.db_idx128_upperbound = db_idx128_upperbound;

      _vm_api.db_idx128_end = db_idx128_end;
      _vm_api.db_idx256_store = db_idx256_store;
      _vm_api.db_idx256_update = db_idx256_update;
      _vm_api.db_idx256_remove = db_idx256_remove;
      _vm_api.db_idx256_next = db_idx256_next;

      _vm_api.db_idx256_previous = db_idx256_previous;
      _vm_api.db_idx256_find_primary = db_idx256_find_primary;
      _vm_api.db_idx256_find_secondary = db_idx256_find_secondary;
      _vm_api.db_idx256_lowerbound = db_idx256_lowerbound;
      _vm_api.db_idx256_upperbound = db_idx256_upperbound;
      _vm_api.db_idx256_end = db_idx256_end;
      _vm_api.db_idx_double_store = db_idx_double_store;
      _vm_api.db_idx_double_update = db_idx_double_update;
      _vm_api.db_idx_double_remove = db_idx_double_remove;
      _vm_api.db_idx_double_next = db_idx_double_next;
      _vm_api.db_idx_double_previous = db_idx_double_previous;
      _vm_api.db_idx_double_find_primary = db_idx_double_find_primary;
      _vm_api.db_idx_double_find_secondary = db_idx_double_find_secondary;
      _vm_api.db_idx_double_lowerbound = db_idx_double_lowerbound;
      _vm_api.db_idx_double_upperbound = db_idx_double_upperbound;
      _vm_api.db_idx_double_end = db_idx_double_end;
      _vm_api.db_idx_long_double_store = db_idx_long_double_store;
      _vm_api.db_idx_long_double_update = db_idx_long_double_update;
      _vm_api.db_idx_long_double_remove = db_idx_long_double_remove;
      _vm_api.db_idx_long_double_next = db_idx_long_double_next;
      _vm_api.db_idx_long_double_previous = db_idx_long_double_previous;
      _vm_api.db_idx_long_double_find_primary = db_idx_long_double_find_primary;
      _vm_api.db_idx_long_double_find_secondary = db_idx_long_double_find_secondary;
      _vm_api.db_idx_long_double_lowerbound = db_idx_long_double_lowerbound;
      _vm_api.db_idx_long_double_upperbound = db_idx_long_double_upperbound;
      _vm_api.db_idx_long_double_end = db_idx_long_double_end;

      _vm_api.check_transaction_authorization = check_transaction_authorization;
      _vm_api.check_permission_authorization = check_permission_authorization;
      _vm_api.get_permission_last_used = get_permission_last_used;
      _vm_api.get_account_creation_time = get_account_creation_time;



      _vm_api.prints = prints;
      _vm_api.prints_l = prints_l;
      _vm_api.printi = printi;
      _vm_api.printui = printui;
      _vm_api.printi128 = printi128;
      _vm_api.printui128 = printui128;
      _vm_api.printsf = printsf;
      _vm_api.printdf = printdf;
      _vm_api.printqf = printqf;
      _vm_api.printn = printn;
      _vm_api.printhex = printhex;

      _vm_api.set_resource_limits = set_resource_limits;
      _vm_api.get_resource_limits = get_resource_limits;
      _vm_api.set_proposed_producers = set_proposed_producers;
      _vm_api.is_privileged = is_privileged;
      _vm_api.set_privileged = set_privileged;
      _vm_api.set_blockchain_parameters_packed = set_blockchain_parameters_packed;
      _vm_api.get_blockchain_parameters_packed = get_blockchain_parameters_packed;
      _vm_api.activate_feature = activate_feature;

      _vm_api.eosio_abort = eosio_abort;
      _vm_api.eosio_assert = eosio_assert;
      _vm_api.eosio_assert_message = eosio_assert_message;
      _vm_api.eosio_assert_code = eosio_assert_code;
      _vm_api.eosio_exit = eosio_exit;
      _vm_api.current_time = current_time;
      _vm_api.now = now;

      _vm_api.checktime = checktime;
      _vm_api.check_context_free = check_context_free;
      _vm_api.contracts_console = contracts_console;
      _vm_api.vm_cleanup = nullptr;
      _vm_api.vm_run_script = nullptr;
      _vm_api.vm_run_lua_script = nullptr;
      _vm_api.vm_cpython_compile = nullptr;
      _vm_api.is_debug_mode = is_debug_mode_;
      _vm_api.is_unittest_mode = is_unittest_mode;
      _vm_api.throw_exception = fc_throw_exception;
      _vm_api.is_producing_block = is_producing_block;

      _vm_api.vm_set_debug_contract = nullptr;
      _vm_api.vm_get_debug_contract = nullptr;

      _vm_api.wasm_to_wast = nullptr;
      _vm_api.wast_to_wasm = nullptr;

      _vm_api.get_wasm_runtime_type = get_wasm_runtime_type;

      _vm_api.log = log_;

      _vm_api.update_db_usage = update_db_usage;
      _vm_api.verify_account_ram_usage = verify_account_ram_usage;

      _vm_api.send_deferred = _send_deferred;
      _vm_api.cancel_deferred = _cancel_deferred;
      _vm_api.read_transaction = read_transaction;
      _vm_api.transaction_size = transaction_size;

      _vm_api.tapos_block_num = tapos_block_num;
      _vm_api.tapos_block_prefix = tapos_block_prefix;
      _vm_api.expiration = expiration;
      _vm_api.get_action = get_action;

      _vm_api.assert_privileged = assert_privileged;
      _vm_api.assert_context_free = assert_context_free;
      _vm_api.get_context_free_data = get_context_free_data;

      _vm_api.get_code = get_code;
      _vm_api.set_code = set_code;
      _vm_api.set_code_ext = set_code_ext;
      _vm_api.load_code_ext = load_code_ext;
      _vm_api.check_code_auth = check_code_auth,

      _vm_api.get_code_id = get_code_id;
      _vm_api.get_code_type = get_code_type;

      _vm_api.rodb_remove_i64 = db_api_remove_i64;

      _vm_api.rodb_find_i64 = db_api_find_i64;
      _vm_api.rodb_get_i64_ex = db_api_get_i64_ex;
      _vm_api.rodb_get_i64_exex = db_api_get_i64_exex;

      _vm_api.rodb_next_i64 = db_api_next_i64;
      _vm_api.rodb_previous_i64 = db_api_previous_i64;
      _vm_api.rodb_lowerbound_i64 = db_api_lowerbound_i64;
      _vm_api.rodb_upperbound_i64 = db_api_upperbound_i64;
      _vm_api.rodb_end_i64 = db_api_end_i64;

      _vm_api.get_action_account = get_action_account;
      _vm_api.string_to_uint64 = string_to_uint64_;
      _vm_api.uint64_to_string = uint64_to_string_;
      _vm_api.string_to_symbol = string_to_symbol_c;
      _vm_api.resume_billing_timer = resume_billing_timer;
      _vm_api.pause_billing_timer = pause_billing_timer;

      _vm_api.wasm_call = wasm_call;

      _vm_api.call_set_args = call_set_args;
      _vm_api.call_get_args = call_get_args;
      _vm_api.call = call;
      _vm_api.get_call_status = get_call_status;
      _vm_api.call_set_results = call_set_results;
      _vm_api.call_get_results = call_get_results;

      _vm_api.has_option = has_option;
      _vm_api.get_option = get_option;
      _vm_api.app_init_finished = app_init_finished;
      _vm_api.run_mode = run_mode;

      _vm_api.ethaddr2n = nullptr;
      _vm_api.n2ethaddr = nullptr;
      _vm_api.is_contracts_console_enabled = is_contracts_console_enabled;
   }
   vm_register_api(&_vm_api);

   _vm_api_cpp.get_current_exception = get_current_exception;
   vm_register_api_cpp(&_vm_api_cpp);
}

extern "C" void vm_api_init_ex() {
   char _path[128];
//   libeosio_native
   vm_register_api(&_vm_api);
   snprintf(_path, sizeof(_path), "../libs/libeosiolib_native%s", DYLIB_SUFFIX);
   void* handle = dlopen(_path, RTLD_LAZY | RTLD_LOCAL);

   if (handle == NULL) {
      elog("loading ${n} failed", ("n", _path));
      return;
   }

   fn_register_vm_api _register_vm_api = (fn_register_vm_api)dlsym(handle, "vm_register_api");
   if (_register_vm_api) {
      _register_vm_api(&_vm_api);
   } else {
      elog("vm_register_api not found!");
   }
}

void register_vm_api(void* handle) {
   fn_register_vm_api _register_vm_api = (fn_register_vm_api)dlsym(handle, "vm_register_api");
   if (_register_vm_api) {
      _register_vm_api(&_vm_api);
   } else {
      elog("vm_register_api not found!");
   }
}

std::istream& operator>>(std::istream& in, wasm_interface::vm_type& runtime) {
   std::string s;
   in >> s;
   if (s == "wavm")
      runtime = eosio::chain::wasm_interface::vm_type::wavm;
   else if (s == "binaryen")
      runtime = eosio::chain::wasm_interface::vm_type::binaryen;
   else if (s == "wabt")
      runtime = eosio::chain::wasm_interface::vm_type::wabt;
   else
      in.setstate(std::ios_base::failbit);
   return in;
}

bool vm_cleanup() {
   return _vm_api.vm_cleanup();
}

bool vm_run_script(const char* str) {
   return _vm_api.vm_run_script(str);
}


}}
