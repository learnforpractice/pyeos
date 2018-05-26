#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "_cgo_export.h"
#include "mpeoslib.h"

static struct eosapi s_eosapi;

//mpeoslib.cpp
typedef struct eosapi* (*fn_get_eosapi)();
void set_get_eosapi_func(fn_get_eosapi fn);


//mpeoslib.cpp
mp_obj_t uint64_to_string_(uint64_t n);

//db_api.cpp
int mp_action_size();
int mp_read_action(char* buf, size_t size);
int mp_is_account(uint64_t account);
uint64_t mp_get_receiver();

void mp_db_get_table_i64( int itr, uint64_t *code, uint64_t *scope, uint64_t *payer, uint64_t *table, uint64_t *id);

void db_api_remove_i64(int itr);
int db_api_get_i64( int itr, char* buffer, size_t buffer_size );
int db_api_next_i64( int itr, uint64_t* primary );
int db_api_previous_i64( int itr, uint64_t* primary );
int db_api_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_api_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_api_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_api_end_i64( uint64_t code, uint64_t scope, uint64_t table );


int call_onApply(uint64_t receiver, uint64_t account, uint64_t act, char** err, int* len) {
   struct onApply_return ret = onApply(receiver, account, act);
   *err = ret.r2;
   *len = ret.r1;
   return ret.r0;
}

static void mp_require_auth(uint64_t account) {
   printf("+++++++++++++++mp_require_auth\n");
}

static int mp_db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   DbStoreI64((GoInt64)scope, (GoInt64)table, (GoInt64)payer, (GoInt64)id, (void *)buffer, (GoInt)buffer_size);
   return db_api_find_i64(mp_get_receiver(), scope, table, id);
}

static void mp_db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
   uint64_t code;
   uint64_t scope;
   uint64_t _payer;
   uint64_t table;
   uint64_t id;
   mp_db_get_table_i64(itr, &code, &scope, &_payer, &table, &id);
   printf("++++++++++++%llu %lu\n", (uint64_t)buffer, buffer_size);
   DbUpdateI64Ex( (GoInt64)scope, (GoInt64)payer, (GoInt64)table, (GoInt64)id, (void *)buffer, (GoInt)buffer_size);
}

static void mp_db_remove_i64( int itr ) {
   uint64_t code;
   uint64_t scope;
   uint64_t payer;
   uint64_t table;
   uint64_t id;
   mp_db_get_table_i64(itr, &code, &scope, &payer, &table, &id);
   DbRemoveI64Ex((GoInt64)scope, (GoInt64)payer, (GoInt64)table, (GoInt64)id);
   db_api_remove_i64(itr);
}

static int mp_db_get_i64( int itr, char* buffer, size_t buffer_size ) {
   return db_api_get_i64(itr, buffer, buffer_size);
}

static int mp_db_next_i64( int itr, uint64_t* primary ) {
   return db_api_next_i64(itr, primary);
}

static int mp_db_previous_i64( int itr, uint64_t* primary ) {
   return db_api_previous_i64(itr, primary);
}

static int mp_db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api_find_i64(code, scope, table, id);
}

static int mp_db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api_lowerbound_i64(code, scope, table, id);
}

static int mp_db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api_upperbound_i64(code, scope, table, id);
}

static int mp_db_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
   return db_api_end_i64(code, scope, table);
}

static struct eosapi* get_eosapi() {
   return &s_eosapi;
}

void mp_init_eosapi() {
   static bool _init = false;
   if (_init) {
      return;
   }
   _init = true;

   s_eosapi.wasm_call = wasm_call_;
//   s_eosapi.string_to_symbol = string_to_symbol;
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
   s_eosapi.sha1 = sha1_;
   s_eosapi.sha256 = sha256_;
   s_eosapi.sha512 = sha512_;
   s_eosapi.ripemd160 = ripemd160_;

   s_eosapi.string_to_uint64_ = string_to_uint64_;
   s_eosapi.uint64_to_string_ = uint64_to_string_;

   s_eosapi.pack_ = pack_;
   s_eosapi.unpack_ = unpack_;

   s_eosapi.action_size = mp_action_size;
   s_eosapi.read_action = mp_read_action;

   s_eosapi.require_auth = mp_require_auth;

   s_eosapi.db_store_i64 = mp_db_store_i64;
   s_eosapi.db_update_i64 = mp_db_update_i64;
   s_eosapi.db_remove_i64 = mp_db_remove_i64;
   s_eosapi.db_get_i64 = mp_db_get_i64;
//   s_eosapi.db_get_i64_ex = mp_db_get_i64_ex;
   s_eosapi.db_next_i64 = mp_db_next_i64;
   s_eosapi.db_previous_i64 = mp_db_previous_i64;
   s_eosapi.db_find_i64 = mp_db_find_i64;
   s_eosapi.db_lowerbound_i64 = mp_db_lowerbound_i64;
   s_eosapi.db_upperbound_i64 = mp_db_upperbound_i64;
   s_eosapi.db_end_i64 = mp_db_end_i64;

   s_eosapi.is_account = mp_is_account;


//mpeoslib.cpp
   s_eosapi.split_path = split_path;

   set_get_eosapi_func(get_eosapi);
}

