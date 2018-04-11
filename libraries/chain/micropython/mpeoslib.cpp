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
#include <eosio/chain/config.hpp>

#include <boost/filesystem.hpp>
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

#include <dlfcn.h>

using namespace eosio;
using namespace eosio::chain;

static struct mpapi s_mpapi;
static struct eosapi s_eosapi;

std::map<std::thread::id, struct mpapi*> api_map;


static int counter = 0;
struct mpapi& get_mpapi() {
   std::thread::id this_id = std::this_thread::get_id();
   auto itr = api_map.find(this_id);
   if ( itr != api_map.end()) {
      return *itr->second;
   }

   char buffer[128];
   counter += 1;

#ifdef __APPLE__
   sprintf(buffer, "../libraries/micropython/libmicropython%d.dylib", counter);
#else
   sprintf(buffer, "../libraries/micropython/libmicropython%d.so", counter);
#endif

   void *handle = dlopen(buffer, RTLD_LAZY | RTLD_LOCAL);

   wlog("${n1}, ${n2}", ("n1", buffer)("n2", (uint64_t)handle));

   assert(handle != NULL);

   fn_mp_register_eosapi mp_register_eosapi = (fn_mp_register_eosapi)dlsym(handle, "mp_register_eosapi");
   fn_mp_obtain_mpapi mp_obtain_mpapi = (fn_mp_obtain_mpapi)dlsym(handle, "mp_obtain_mpapi");

   fn_main_micropython main_micropython = (fn_main_micropython)dlsym(handle, "main_micropython");

   struct mpapi* api = new mpapi();
   mp_register_eosapi(&s_eosapi);
   main_micropython(0, NULL);

   mp_obtain_mpapi(api);
   api_map[this_id] = api;
   return *api;
}

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
   return get_mpapi().mp_obj_new_str(s.c_str(), s.size());
}

mp_obj_t pack_(const char* str, int nsize) {
   string raw(str,nsize);
   std::vector<char> o = fc::raw::pack<string>(raw);
   string out = string(o.begin(), o.end());
   return get_mpapi().mp_obj_new_str(out.c_str(), out.size());
}

mp_obj_t unpack_(const char* str, int nsize) {
   string raw(str,nsize);
   std::vector<char> v(raw.begin(), raw.end());
   string out = fc::raw::unpack<string>(v);
   return get_mpapi().mp_obj_new_str(out.c_str(), out.size());
}

}

using namespace eosio::chain;
using namespace eosio::chain::contracts;

namespace eosio { namespace micropython {


static inline apply_context& ctx() {
   return *get_current_context();
}

}
}

using namespace eosio::micropython;

extern "C" {


int db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   return ctx().db_store_i64( scope, table, payer, id, buffer, buffer_size );
}
void db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
   ctx().db_update_i64( itr, payer, buffer, buffer_size );
}
void db_remove_i64( int itr ) {
   ctx().db_remove_i64( itr );
}
int db_get_i64( int itr, char* buffer, size_t buffer_size ) {
   return ctx().db_get_i64( itr, buffer, buffer_size );
}
int db_next_i64( int itr, uint64_t* primary ) {
   return ctx().db_next_i64(itr, *primary);
}
int db_previous_i64( int itr, uint64_t* primary ) {
   return ctx().db_previous_i64(itr, *primary);
}
int db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return ctx().db_find_i64( code, scope, table, id );
}
int db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return ctx().db_lowerbound_i64( code, scope, table, id );
}
int db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return ctx().db_upperbound_i64( code, scope, table, id );
}
int db_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
   return ctx().db_end_i64( code, scope, table );
}

#define DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(IDX, TYPE)\
      int db_##IDX##_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* secondary , size_t len) {\
         FC_ASSERT(len == sizeof(TYPE), "bad length"); \
         try { \
            return ctx().IDX.store( scope, table, payer, id, *((TYPE*)secondary) );\
         } catch (...) { \
            return -1; \
         } \
      }\
      void db_##IDX##_update( int iterator, uint64_t payer, const char* secondary , size_t len ) {\
         FC_ASSERT(len == sizeof(TYPE), "bad length"); \
         return ctx().IDX.update( iterator, payer, *((TYPE*)secondary) );\
      }\
      void db_##IDX##_remove( int iterator ) {\
         return ctx().IDX.remove( iterator );\
      }\
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const char* secondary , size_t len, uint64_t* primary ) {\
         FC_ASSERT(len == sizeof(TYPE), "bad length"); \
         return ctx().IDX.find_secondary(code, scope, table, *((TYPE*)secondary), *primary);\
      }\
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t primary ) {\
         FC_ASSERT(len == sizeof(TYPE), "bad length"); \
         return ctx().IDX.find_primary(code, scope, table, *((TYPE*)secondary), primary);\
      }\
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  char* secondary , size_t len, uint64_t* primary ) {\
         FC_ASSERT(len == sizeof(TYPE), "bad length"); \
         return ctx().IDX.lowerbound_secondary(code, scope, table, *((TYPE*)secondary), *primary);\
      }\
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table,  char* secondary , size_t len, uint64_t* primary ) {\
         FC_ASSERT(len == sizeof(TYPE), "bad length"); \
         return ctx().IDX.upperbound_secondary(code, scope, table, *((TYPE*)secondary), *primary);\
      }\
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return ctx().IDX.end_secondary(code, scope, table);\
      }\
      int db_##IDX##_next( int iterator, uint64_t* primary  ) {\
         return ctx().IDX.next_secondary(iterator, *primary);\
      }\
      int db_##IDX##_previous( int iterator, uint64_t* primary ) {\
         return ctx().IDX.previous_secondary(iterator, *primary);\
      }

#define DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY(IDX, ARR_SIZE, ARR_ELEMENT_TYPE)\
      int db_##IDX##_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* data, size_t data_len) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return ctx().IDX.store(scope, table, payer, id, (ARR_ELEMENT_TYPE*)data);\
      }\
      void db_##IDX##_update( int iterator, uint64_t payer, const char* data, size_t data_len ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return ctx().IDX.update(iterator, payer, (ARR_ELEMENT_TYPE*)data);\
      }\
      void db_##IDX##_remove( int iterator ) {\
         return ctx().IDX.remove(iterator);\
      }\
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const char* data, size_t data_len, uint64_t* primary ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return ctx().IDX.find_secondary(code, scope, table, (ARR_ELEMENT_TYPE*)data, *primary);\
      }\
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, char* data, size_t data_len, uint64_t primary ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return ctx().IDX.find_primary(code, scope, table, (ARR_ELEMENT_TYPE*)data, primary);\
      }\
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table, char* data, size_t data_len, uint64_t* primary ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return ctx().IDX.lowerbound_secondary(code, scope, table, (ARR_ELEMENT_TYPE*)data, *primary);\
      }\
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table, char* data, size_t data_len, uint64_t* primary ) {\
         FC_ASSERT( data_len == sizeof(ARR_ELEMENT_TYPE)*ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return ctx().IDX.upperbound_secondary(code, scope, table, (ARR_ELEMENT_TYPE*)data, *primary);\
      }\
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return ctx().IDX.end_secondary(code, scope, table);\
      }\
      int db_##IDX##_next( int iterator, uint64_t* primary  ) {\
         return ctx().IDX.next_secondary(iterator, *primary);\
      }\
      int db_##IDX##_previous( int iterator, uint64_t* primary ) {\
         return ctx().IDX.previous_secondary(iterator, *primary);\
      }

DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(idx64,  uint64_t)
DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(idx128, eosio::chain::uint128_t)
DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY(idx256, 2, eosio::chain::uint128_t)
DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(idx_double, uint64_t)


///////////////////////////////////////////////////////////////////////////////////////////////////
//context_free_transaction_api
int read_transaction( char* data, size_t data_len ) {
   bytes trx = ctx().get_packed_transaction();
   if (data_len >= trx.size()) {
      memcpy(data, trx.data(), trx.size());
   }
   return trx.size();
}

int transaction_size() {
   return ctx().get_packed_transaction().size();
}

int expiration() {
  return ctx().trx_meta.trx().expiration.sec_since_epoch();
}

int tapos_block_num() {
  return ctx().trx_meta.trx().ref_block_num;
}
int tapos_block_prefix() {
  return ctx().trx_meta.trx().ref_block_prefix;
}

int get_action( uint32_t type, uint32_t index, char* buffer, size_t buffer_size ) {
   return ctx().get_action( type, index, buffer, buffer_size );
}

///////////////////////////////////////////////////////////////////////////
//action_api

int read_action(char* memory, size_t size) {
   FC_ASSERT(size > 0);
   int minlen = std::min<size_t>(ctx().act.data.size(), size);
   memcpy((void *)memory, ctx().act.data.data(), minlen);
   return minlen;
}

int action_size() {
   return ctx().act.data.size();
}

uint64_t current_receiver() {
   return ctx().receiver.value;
}

uint64_t publication_time() {
   return ctx().trx_meta.published.time_since_epoch().count();
}

uint64_t current_sender() {
   if (ctx().trx_meta.sender) {
      return (*ctx().trx_meta.sender).value;
   } else {
      return 0;
   }
}

//apply_context
void require_auth(uint64_t account) {
   ctx().require_authorization(account_name(account));
}

void require_auth_ex(uint64_t account, uint64_t permission) {
   ctx().require_authorization(account_name(account), name(permission));
}

void require_write_lock(uint64_t scope) {
   ctx().require_write_lock(name(scope));
}

void require_read_lock(uint64_t account, uint64_t scope) {
   ctx().require_read_lock(name(account), name(scope));
}

int is_account(uint64_t account) {
   return ctx().is_account(name(account));
}

void require_recipient(uint64_t account) {
   ctx().require_recipient(name(account));
}

//producer_api
int get_active_producers(uint64_t* producers, size_t datalen) {
   auto active_producers = ctx().get_active_producers();
   size_t len = active_producers.size();
   size_t cpy_len = std::min(datalen, len);
   memcpy(producers, active_producers.data(), cpy_len * sizeof(chain::account_name) );
   return len;
}


#if 0
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
   ctx().execute_inline(std::move(act));
   return mp_obj_new_int(0);
}
#endif

bool unpack_action(struct mp_action* mp_act , action& act) {
   act.account = name(mp_act->account);
   act.name = name(mp_act->name);
   act.data.resize(0);
   act.data.resize(mp_act->data_len);
   memcpy(act.data.data(), mp_act->data, mp_act->data_len);

   for (int i=0;i<mp_act->auth_len;i+=2) {
      account_name    actor = name(mp_act->auth[i]);
      permission_name permission = name(mp_act->auth[i+1]);
      act.authorization.push_back({actor, permission});
   }
   return true;
}

bool unpack_transaction(struct mp_transaction* mp_trx, transaction& trx) {
   if (mp_trx == NULL) {
      return false;
   }

   trx.expiration             = time_point_sec(mp_trx->expiration);
   trx.region                 = mp_trx->region;
   trx.ref_block_num          = mp_trx->ref_block_num;
   trx.ref_block_prefix       = mp_trx->ref_block_prefix;
   trx.max_net_usage_words    = mp_trx->max_net_usage_words;
   trx.max_kcpu_usage         = mp_trx->max_kcpu_usage;
   trx.delay_sec              = mp_trx->delay_sec;

   for (int i=0;i<mp_trx->free_actions_len;i++) {
      action act;
      unpack_action(mp_trx->context_free_actions[i], act);
      trx.context_free_actions.emplace_back(std::move(act));
   }

   for (int i=0;i<mp_trx->actions_len;i++) {
      action act;
      unpack_action(mp_trx->actions[i], act);
      trx.actions.emplace_back(std::move(act));
   }
   return true;

}

void send_inline( struct mp_action* mp_act ) {
   // TODO: use global properties object for dynamic configuration of this default_max_gen_trx_size
   FC_ASSERT( mp_act && mp_act->data_len < config::default_max_inline_action_size, "inline action too big" );

   action act;
   unpack_action(mp_act, act);

   ctx().execute_inline(std::move(act));
}

void send_context_free_inline(struct mp_action* mp_act) {
   // TODO: use global properties object for dynamic configuration of this default_max_gen_trx_size
   FC_ASSERT( mp_act && mp_act->data_len < config::default_max_inline_action_size, "inline action too big" );

   action act;
   unpack_action(mp_act, act);

   ctx().execute_context_free_inline(std::move(act));
}

void send_deferred( eosio::chain::uint128_t sender_id, uint64_t payer, struct mp_transaction* mp_trx ) {
   deferred_transaction dtrx;
   try {
      unpack_transaction(mp_trx, dtrx);

      dtrx.sender = ctx().receiver;
      dtrx.sender_id = sender_id;
      dtrx.execute_after = time_point_sec( (ctx().controller.head_block_time() + fc::seconds(dtrx.delay_sec)) + fc::microseconds(999'999) ); // rounds up to nearest second
            dtrx.payer = payer;
      ctx().execute_deferred(std::move(dtrx));
   } FC_CAPTURE_AND_RETHROW((fc::to_hex((char*)&dtrx, sizeof(dtrx))));
}

void cancel_deferred( eosio::chain::uint128_t  val ) {
   fc::uint128_t sender_id(val>>64, uint64_t(val) );
   ctx().cancel_deferred( (eosio::chain::uint128_t)sender_id );
}

uint32_t now() {
   auto& ctrl = ctx().controller;
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

   return get_mpapi().mp_obj_new_str(pub, pubds.tellp());
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
   return get_mpapi().mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

mp_obj_t sha256(const char* data, size_t datalen) {
   string str_hash = fc::sha256::hash( data, datalen ).str();
   return get_mpapi().mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

mp_obj_t sha512(const char* data, size_t datalen) {
   string str_hash = fc::sha512::hash( data, datalen ).str();
   return get_mpapi().mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

mp_obj_t ripemd160(const char* data, size_t datalen) {
   string str_hash = fc::ripemd160::hash( data, datalen ).str();
   return get_mpapi().mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

uint64_t get_action_account() {
   return ctx().act.account.value;
}


#if 0
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
#endif

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__)
  #include <windows.h>
  extern "C" void eosio_delay(int ms) {
     Sleep( ms );
  }
#else  /* presume POSIX */
  extern "C" void eosio_delay(int ms) {
     usleep(ms*1000);
  }
#endif

int split_path(const char* str_path, char *path1, size_t path1_size, char *path2, size_t path2_size) {
   std::vector<std::string> v;
   const boost::filesystem::path _path(str_path);
   for (const auto &p : _path) {
       v.push_back(p.filename().string());
   }

   if (v.size() >= 1) {
      if (path1_size > v[0].size()) {
         strncpy(path1, v[0].c_str(), v[0].size());
      } else {
         strncpy(path1, v[0].c_str(), path1_size);
      }
   }

   if (v.size() >= 2) {
      if (path1_size > v[1].size()) {
         strncpy(path2, v[1].c_str(), v[1].size());
      } else {
         strncpy(path2, v[1].c_str(), path1_size);
      }
   }

   return v.size();
}

void init_eosapi() {
   static bool _init = false;
   if (_init) {
      return;
   }
   _init = true;
   s_eosapi.string_to_symbol = string_to_symbol;
   s_eosapi.eosio_delay = eosio_delay;
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

   s_eosapi.db_idx64_store = db_idx64_store;
   s_eosapi.db_idx64_update = db_idx64_update;
   s_eosapi.db_idx64_remove = db_idx64_remove;
   s_eosapi.db_idx64_find_secondary = db_idx64_find_secondary;
   s_eosapi.db_idx64_find_primary = db_idx64_find_primary;
   s_eosapi.db_idx64_lowerbound = db_idx64_lowerbound;
   s_eosapi.db_idx64_upperbound = db_idx64_upperbound;
   s_eosapi.db_idx64_end = db_idx64_end;
   s_eosapi.db_idx64_next = db_idx64_next;
   s_eosapi.db_idx64_previous = db_idx64_previous;

   s_eosapi.db_idx128_store = db_idx128_store;
   s_eosapi.db_idx128_update = db_idx128_update;
   s_eosapi.db_idx128_remove = db_idx128_remove;
   s_eosapi.db_idx128_find_secondary = db_idx128_find_secondary;
   s_eosapi.db_idx128_find_primary = db_idx128_find_primary;
   s_eosapi.db_idx128_lowerbound = db_idx128_lowerbound;
   s_eosapi.db_idx128_upperbound = db_idx128_upperbound;
   s_eosapi.db_idx128_end = db_idx128_end;
   s_eosapi.db_idx128_next = db_idx128_next;
   s_eosapi.db_idx128_previous = db_idx128_previous;

   s_eosapi.db_idx_double_store = db_idx_double_store;
   s_eosapi.db_idx_double_update = db_idx_double_update;
   s_eosapi.db_idx_double_remove = db_idx_double_remove;
   s_eosapi.db_idx_double_find_secondary = db_idx_double_find_secondary;
   s_eosapi.db_idx_double_find_primary = db_idx_double_find_primary;
   s_eosapi.db_idx_double_lowerbound = db_idx_double_lowerbound;
   s_eosapi.db_idx_double_upperbound = db_idx_double_upperbound;
   s_eosapi.db_idx_double_end = db_idx_double_end;
   s_eosapi.db_idx_double_next = db_idx_double_next;
   s_eosapi.db_idx_double_previous = db_idx_double_previous;

   s_eosapi.db_idx256_store = db_idx256_store;
   s_eosapi.db_idx256_update = db_idx256_update;
   s_eosapi.db_idx256_remove = db_idx256_remove;
   s_eosapi.db_idx256_find_secondary = db_idx256_find_secondary;
   s_eosapi.db_idx256_find_primary = db_idx256_find_primary;
   s_eosapi.db_idx256_lowerbound = db_idx256_lowerbound;
   s_eosapi.db_idx256_upperbound = db_idx256_upperbound;
   s_eosapi.db_idx256_end = db_idx256_end;
   s_eosapi.db_idx256_next = db_idx256_next;
   s_eosapi.db_idx256_previous = db_idx256_previous;

   s_eosapi.send_inline = send_inline;
   s_eosapi.send_context_free_inline = send_context_free_inline;
   s_eosapi.send_deferred = send_deferred;
   s_eosapi.cancel_deferred = cancel_deferred;

   s_eosapi.split_path = split_path;
   s_eosapi.get_action_account = get_action_account;
}

void init_api() {
   init_eosapi();
   get_mpapi();
}


}


