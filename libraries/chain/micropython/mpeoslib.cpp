#include "mpeoslib.h"

#include <eosio/chain/chain_controller.hpp>
#include <eosio/chain/micropython_interface.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/raw.hpp>



//#include <eosio/chain/wasm_interface.hpp>
#include <eosio/chain/apply_context.hpp>
#include <eosio/chain/chain_controller.hpp>
#include <eosio/chain/producer_schedule.hpp>
#include <eosio/chain/asset.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/types.hpp>

#include <boost/core/ignore_unused.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
//#include <eosio/chain/wasm_interface_private.hpp>
//#include <eosio/chain/wasm_eosio_constraints.hpp>
#include <fc/exception/exception.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/sha1.hpp>
#include <fc/io/raw.hpp>
#include <fc/utf8.hpp>

#include <Runtime/Runtime.h>
#include "IR/Module.h"
#include "Platform/Platform.h"
#include "WAST/WAST.h"
#include "IR/Operators.h"
#include "IR/Validate.h"
#include "IR/Types.h"
#include "Runtime/Runtime.h"
#include "Runtime/Linker.h"
#include "Runtime/Intrinsics.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <mutex>
#include <thread>
#include <condition_variable>





using namespace eosio;
using namespace eosio::chain;

extern "C" {
//typedef long long int64_t;
//typedef unsigned long long uint64_t;

typedef uint64_t TableName;

uint64_t string_to_uint64_(const char* str) {
   try {
      return name(str).value;
   } catch (...) {
   }
   return 0;
}

mp_obj_t uint64_to_string_(uint64_t n) {
   string s = name(n).to_string();
//   ilog("n: ${n}",("n", s.size()));
   return mp_obj_new_str(s.c_str(), s.size());
}

mp_obj_t pack_(const char* str, int nsize) {
   string raw(str,nsize);
   std::vector<char> o = fc::raw::pack<string>(raw);
   string out = string(o.begin(), o.end());
   return mp_obj_new_str(out.c_str(), out.size());
}

mp_obj_t unpack_(const char* str, int nsize) {
   string raw(str,nsize);
   std::vector<char> v(raw.begin(), raw.end());
   string out = fc::raw::unpack<string>(v);
   return mp_obj_new_str(out.c_str(), out.size());
}

}

using namespace eosio::chain;
using namespace eosio::chain::contracts;

namespace eosio { namespace micropython {


static inline apply_context& get_apply_ctx() {
   return *get_current_context();
}


#define DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(IDX, TYPE)\
      int db_##IDX##_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const TYPE& secondary ) {\
         return context->IDX.store( scope, table, payer, id, secondary );\
      }\
      void db_##IDX##_update( int iterator, uint64_t payer, const TYPE& secondary ) {\
         return context->IDX.update( iterator, payer, secondary );\
      }\
      void db_##IDX##_remove( int iterator ) {\
         return context->IDX.remove( iterator );\
      }\
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const TYPE& secondary, uint64_t& primary ) {\
         return context->IDX.find_secondary(code, scope, table, secondary, primary);\
      }\
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, TYPE& secondary, uint64_t primary ) {\
         return context->IDX.find_primary(code, scope, table, secondary, primary);\
      }\
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE& secondary, uint64_t& primary ) {\
         return context->IDX.lowerbound_secondary(code, scope, table, secondary, primary);\
      }\
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE& secondary, uint64_t& primary ) {\
         return context->IDX.upperbound_secondary(code, scope, table, secondary, primary);\
      }\
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return context->IDX.end_secondary(code, scope, table);\
      }\
      int db_##IDX##_next( int iterator, uint64_t& primary  ) {\
         return context->IDX.next_secondary(iterator, primary);\
      }\
      int db_##IDX##_previous( int iterator, uint64_t& primary ) {\
         return context->IDX.previous_secondary(iterator, primary);\
      }

#define DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY(IDX, ARR_SIZE, ARR_ELEMENT_TYPE)\
      int db_##IDX##_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* data, size_t data_len) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return context->IDX.store(scope, table, payer, id, (ARR_ELEMENT_TYPE*)data);\
      }\
      void db_##IDX##_update( int iterator, uint64_t payer, const char* data, size_t data_len ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return context->IDX.update(iterator, payer, (ARR_ELEMENT_TYPE*)data);\
      }\
      void db_##IDX##_remove( int iterator ) {\
         return context->IDX.remove(iterator);\
      }\
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const char* data, size_t data_len, uint64_t& primary ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return context->IDX.find_secondary(code, scope, table, (ARR_ELEMENT_TYPE*)data, primary);\
      }\
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, char* data, size_t data_len, uint64_t primary ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return context->IDX.find_primary(code, scope, table, (ARR_ELEMENT_TYPE*)data, primary);\
      }\
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table, char* data, size_t data_len, uint64_t& primary ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return context->IDX.lowerbound_secondary(code, scope, table, (ARR_ELEMENT_TYPE*)data, primary);\
      }\
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table, char* data, size_t data_len, uint64_t& primary ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return context->IDX.upperbound_secondary(code, scope, table, (ARR_ELEMENT_TYPE*)data, primary);\
      }\
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return context->IDX.end_secondary(code, scope, table);\
      }\
      int db_##IDX##_next( int iterator, uint64_t& primary  ) {\
         return context->IDX.next_secondary(iterator, primary);\
      }\
      int db_##IDX##_previous( int iterator, uint64_t& primary ) {\
         return context->IDX.previous_secondary(iterator, primary);\
      }

class database_api {
   public:
//      using context_aware_api::context_aware_api;

      apply_context*     context;

      database_api(apply_context& ctx) : context(&ctx) {}

      static database_api& get() {
         static database_api* instance = nullptr;
         if (!instance) {
            instance = new database_api(*get_current_context());
         } else {
            instance->context = get_current_context();
         }
         return *instance;
      }

      int db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
         return context->db_store_i64( scope, table, payer, id, buffer, buffer_size );
      }
      void db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
         context->db_update_i64( itr, payer, buffer, buffer_size );
      }
      void db_remove_i64( int itr ) {
         context->db_remove_i64( itr );
      }
      int db_get_i64( int itr, char* buffer, size_t buffer_size ) {
         return context->db_get_i64( itr, buffer, buffer_size );
      }
      int db_next_i64( int itr, uint64_t* primary ) {
         return context->db_next_i64(itr, *primary);
      }
      int db_previous_i64( int itr, uint64_t* primary ) {
         return context->db_previous_i64(itr, *primary);
      }
      int db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
         return context->db_find_i64( code, scope, table, id );
      }
      int db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
         return context->db_lowerbound_i64( code, scope, table, id );
      }
      int db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
         return context->db_upperbound_i64( code, scope, table, id );
      }
      int db_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
         return context->db_end_i64( code, scope, table );
      }

      DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(idx64,  uint64_t)
      DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(idx128, eosio::chain::uint128_t)
      DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY(idx256, 2, eosio::chain::uint128_t)
      DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(idx_double, uint64_t)
};



}
}

using namespace eosio::micropython;

extern "C" {

int db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   return database_api::get().db_store_i64( scope, table, payer, id, buffer, buffer_size );
}

void db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
   database_api::get().db_update_i64( itr, payer, buffer, buffer_size );
}

void db_remove_i64( int itr ) {
   database_api::get().db_remove_i64( itr );
}

int db_get_i64( int itr, char* buffer, size_t buffer_size ) {
   return database_api::get().db_get_i64( itr, buffer, buffer_size );
}

int db_next_i64( int itr, uint64_t* primary ) {
   return database_api::get().db_next_i64(itr, primary);
}

int db_previous_i64( int itr, uint64_t* primary ) {
   return database_api::get().db_previous_i64(itr, primary);
}

int db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return database_api::get().db_find_i64( code, scope, table, id );
}

int db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return database_api::get().db_lowerbound_i64( code, scope, table, id );
}

int db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return database_api::get().db_upperbound_i64( code, scope, table, id );
}

int db_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
   return database_api::get().db_end_i64( code, scope, table );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//context_free_transaction_api
int read_transaction( char* data, size_t data_len ) {
   bytes trx = get_apply_ctx().get_packed_transaction();
   if (data_len >= trx.size()) {
      memcpy(data, trx.data(), trx.size());
   }
   return trx.size();
}

int transaction_size() {
   return get_apply_ctx().get_packed_transaction().size();
}

int expiration() {
  return get_apply_ctx().trx_meta.trx().expiration.sec_since_epoch();
}

int tapos_block_num() {
  return get_apply_ctx().trx_meta.trx().ref_block_num;
}
int tapos_block_prefix() {
  return get_apply_ctx().trx_meta.trx().ref_block_prefix;
}

int get_action( uint32_t type, uint32_t index, char* buffer, size_t buffer_size ) {
   return get_apply_ctx().get_action( type, index, buffer, buffer_size );
}

///////////////////////////////////////////////////////////////////////////
//action_api

int read_action(char* memory, size_t size) {
   FC_ASSERT(size > 0);
   int minlen = std::min<size_t>(get_apply_ctx().act.data.size(), size);
   memcpy((void *)memory, get_apply_ctx().act.data.data(), minlen);
   return minlen;
}

int action_size() {
   return get_apply_ctx().act.data.size();
}

uint64_t current_receiver() {
   return get_apply_ctx().receiver.value;
}

uint64_t publication_time() {
   return get_apply_ctx().trx_meta.published.time_since_epoch().count();
}

uint64_t current_sender() {
   if (get_apply_ctx().trx_meta.sender) {
      return (*get_apply_ctx().trx_meta.sender).value;
   } else {
      return 0;
   }
}

//apply_context
void require_auth(uint64_t account) {
   get_apply_ctx().require_authorization(account_name(account));
}

void require_auth_ex(uint64_t account, uint64_t permission) {
   get_apply_ctx().require_authorization(account_name(account), name(permission));
}

void require_write_lock(uint64_t scope) {
   get_apply_ctx().require_write_lock(name(scope));
}

void require_read_lock(uint64_t account, uint64_t scope) {
   get_apply_ctx().require_read_lock(name(account), name(scope));
}

int is_account(uint64_t account) {
   return get_apply_ctx().is_account(name(account));
}

void require_recipient(uint64_t account) {
   get_apply_ctx().require_recipient(name(account));
}

//producer_api
int get_active_producers(uint64_t* producers, size_t datalen) {
   auto active_producers = get_apply_ctx().get_active_producers();
   size_t len = active_producers.size();
   size_t cpy_len = std::min(datalen, len);
   memcpy(producers, active_producers.data(), cpy_len * sizeof(chain::account_name) );
   return len;
}


extern "C" mp_obj_t send_inline(size_t n_args, const mp_obj_t *args) {
   size_t len = 0;
   action act;

   const char* account = (const char *)mp_obj_str_get_data(args[0], &len);
   if (!account) {
      return mp_const_none;
   }
   len = 0;
   char* action_name = (char *)mp_obj_str_get_data(args[1], &len);
   if (!action_name) {
      return mp_const_none;
   }
   act.account = account;
   act.name = action_name;
   mp_map_t *map = mp_obj_dict_get_map(args[2]);
   for (size_t i = 0; i < map->alloc; i++) {
       if (MP_MAP_SLOT_IS_FILLED(map, i)) {
           // the key must be a qstr, so intern it if it's a string
           const char* key = mp_obj_str_get_str(map->table[i].key);
           const char* value = mp_obj_str_get_str(map->table[i].value);
           permission_level per = {name(key), name(value)};
           act.authorization.emplace_back(per);
       }
   }
   len = 0;
   char* data = (char *)mp_obj_str_get_data(args[3], &len);
   act.data = bytes(data, data+len);
   get_apply_ctx().execute_inline(std::move(act));
   return mp_obj_new_int(0);
}

#if 0
void send_inline( array_ptr<char> data, size_t data_len ) {
   // TODO: use global properties object for dynamic configuration of this default_max_gen_trx_size
   FC_ASSERT( data_len < config::default_max_inline_action_size, "inline action too big" );

   action act;
   fc::raw::unpack<action>(data, data_len, act);
   context.execute_inline(std::move(act));
}

void send_deferred( uint32_t sender_id, const fc::time_point_sec& execute_after, array_ptr<char> data, size_t data_len ) {
   try {
   // TODO: use global properties object for dynamic configuration of this default_max_gen_trx_size
      FC_ASSERT(data_len < config::default_max_gen_trx_size, "generated transaction too big");

      deferred_transaction dtrx;
      fc::raw::unpack<transaction>(data, data_len, dtrx);
      dtrx.sender = context.receiver;
      dtrx.sender_id = sender_id;
      dtrx.execute_after = execute_after;
      context.execute_deferred(std::move(dtrx));
   } FC_CAPTURE_AND_RETHROW((fc::to_hex(data, data_len)));
}
#endif

uint32_t now() {
   auto& ctrl = get_apply_ctx().controller;
   return ctrl.head_block_time().sec_since_epoch();
}

void eosio_abort() {
   edump(("abort() called"));
   FC_ASSERT( false, "abort() called");
}

void eosio_assert(int condition, const char* str) {
   std::string message( str );
   if( !condition ) edump((message));
   FC_ASSERT( condition, "assertion failed: ${s}", ("s",message));
}

//class crypto_api
void assert_recover_key( const char* data, size_t data_len, const char* sig, size_t siglen, const char* pub, size_t publen ) {
   fc::sha256 digest( data, data_len);
   fc::crypto::signature s;
   fc::crypto::public_key p;
   fc::datastream<const char*> ds( sig, siglen );
   fc::datastream<const char*> pubds( pub, publen );

   fc::raw::unpack(ds, s);
   fc::raw::unpack(pubds, p);

   auto check = fc::crypto::public_key( s, digest, false );
   FC_ASSERT( check == p, "Error expected key different than recovered key" );
}

mp_obj_t recover_key(const char* data, size_t size, const char* sig, size_t siglen ) {
   char pub[256];
   fc::sha256 digest(data, size);
   fc::crypto::signature s;
   fc::datastream<const char*> ds( sig, siglen );
   fc::datastream<char*> pubds( pub, sizeof(pub) );

   fc::raw::unpack(ds, s);
   fc::raw::pack( pubds, fc::crypto::public_key( s, digest, false ) );

   return mp_obj_new_str(pub, pubds.tellp());
}

void assert_sha256(const char* data, size_t datalen, const char* hash, size_t hash_len) {
   auto result = fc::sha256::hash( data, datalen );
   fc::sha256 hash_val( hash, hash_len );
   FC_ASSERT( result == hash_val, "hash miss match" );
}

void assert_sha1(const char* data, size_t datalen, const char* hash, size_t hash_len) {
   auto result = fc::sha1::hash( data, datalen );
   fc::sha1 hash_val( string(hash, hash_len) );
   FC_ASSERT( result == hash_val, "hash miss match" );
}

void assert_sha512(const char* data, size_t datalen, const char* hash, size_t hash_len) {
   auto result = fc::sha512::hash( data, datalen );
   fc::sha512 hash_val( string(hash, hash_len) );
   FC_ASSERT( result == hash_val, "hash miss match" );
}

void assert_ripemd160(const char* data, size_t datalen, const char* hash, size_t hash_len) {
   auto result = fc::ripemd160::hash( data, datalen );
   fc::ripemd160 hash_val( string(hash, hash_len) );
   FC_ASSERT( result == hash_val, "hash miss match" );
}

mp_obj_t sha1(const char* data, size_t datalen) {
   string str_hash = fc::sha1::hash( data, datalen ).str();
   return mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

mp_obj_t sha256(const char* data, size_t datalen) {
   string str_hash = fc::sha256::hash( data, datalen ).str();
   return mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

mp_obj_t sha512(const char* data, size_t datalen) {
   string str_hash = fc::sha512::hash( data, datalen ).str();
   return mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

mp_obj_t ripemd160(const char* data, size_t datalen) {
   string str_hash = fc::ripemd160::hash( data, datalen ).str();
   return mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

uint64_t get_action_account() {
   return get_apply_ctx().act.account.value;
}


uint64_t string_to_symbol( uint8_t precision, const char* str ) {
   uint32_t len = 0;
   while( str[len] ) ++len;

   uint64_t result = 0;
   for( uint32_t i = 0; i < len; ++i ) {
      if( str[i] < 'A' || str[i] > 'Z' ) {
         /// ERRORS?
      } else {
         result |= (uint64_t(str[i]) << (8*(1+i)));
      }
   }

   result |= uint64_t(precision);
   return result;
}

static struct eosapi s_eosapi;

void init_eosapi() {
   s_eosapi.now = now;
   s_eosapi.abort_ = abort;
   s_eosapi.eosio_assert = eosio_assert;
   s_eosapi.assert_recover_key = assert_recover_key;

   s_eosapi.recover_key = recover_key;
   s_eosapi.assert_sha256 = assert_sha256;
   s_eosapi.assert_sha1 = assert_sha1;
   s_eosapi.assert_sha512 = assert_sha512;
   s_eosapi.assert_ripemd160 = assert_ripemd160;
   s_eosapi.sha1 = sha1;
   s_eosapi.sha256 = sha256;
   s_eosapi.sha512 = sha512;
   s_eosapi.ripemd160 = ripemd160;

   s_eosapi.string_to_uint64_ = string_to_uint64_;
   s_eosapi.uint64_to_string_ = uint64_to_string_;

   s_eosapi.pack_ = pack_;
   s_eosapi.unpack_ = unpack_;

//   s_eosapi.get_account_balance_
//   s_eosapi.get_active_producers_

   s_eosapi.read_transaction = read_transaction;
   s_eosapi.transaction_size = transaction_size;
   s_eosapi.expiration = expiration;
   s_eosapi.tapos_block_num = tapos_block_num;
   s_eosapi.tapos_block_prefix = tapos_block_prefix;
   s_eosapi.get_action = get_action;


   s_eosapi.require_auth = require_auth;
   s_eosapi.require_auth_ex = require_auth_ex;
   s_eosapi.require_write_lock = require_write_lock;
   s_eosapi.require_read_lock = require_read_lock;
   s_eosapi.is_account = is_account;
   s_eosapi.require_recipient = require_recipient;


   s_eosapi.read_action = read_action;
   s_eosapi.action_size = action_size;
   s_eosapi.current_receiver = current_receiver;
   s_eosapi.publication_time = publication_time;
   s_eosapi.current_sender = current_sender;

   s_eosapi.db_store_i64 = db_store_i64;
   s_eosapi.db_update_i64 = db_update_i64;
   s_eosapi.db_remove_i64 = db_remove_i64;
   s_eosapi.db_get_i64 = db_get_i64;
   s_eosapi.db_next_i64 = db_next_i64;
   s_eosapi.db_previous_i64 = db_previous_i64;
   s_eosapi.db_find_i64 = db_find_i64;
   s_eosapi.db_lowerbound_i64 = db_lowerbound_i64;
   s_eosapi.db_upperbound_i64 = db_upperbound_i64;
   s_eosapi.db_end_i64 = db_end_i64;
}




}
