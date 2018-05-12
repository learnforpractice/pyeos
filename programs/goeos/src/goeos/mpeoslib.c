#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "_cgo_export.h"
#include "mpeoslib.h"

//mpeoslib.cpp
mp_obj_t uint64_to_string_(uint64_t n);
struct mpapi* c_get_mpapi();

int micropython_on_apply(uint64_t receiver, uint64_t account, uint64_t act);

int call_onApply(uint64_t receiver, uint64_t code, uint64_t act)
{
   return micropython_on_apply(receiver, code, act);
}

int read_action(char* buf, size_t size) {
   GoSlice ret = ReadAction();
   if (size > ret.len) {
      size = ret.len;
   }
   memcpy(buf, ret.data, size);
   return size;
}

int mp_db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   GoSlice buf;
   buf.data = (void*)buffer;
   buf.len = buffer_size;
   buf.cap = buffer_size;
   return DbStoreI64((GoInt64)scope, (GoInt64)table, (GoInt64)payer, (GoInt64)id, buf);
}

void mp_db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
   GoSlice buf;
   buf.data = (void*)buffer;
   buf.len = buffer_size;
   buf.cap = buffer_size;
   return DbUpdateI64(itr, payer, buf);
}

void mp_db_remove_i64( int itr ) {
   DbRemoveI64(itr);
}

int mp_db_get_i64( int itr, char* buffer, size_t buffer_size ) {
   GoSlice ret = DbGetI64(itr);
   if (buffer_size > ret.len) {
      buffer_size = ret.len;
   }
   memcpy(buffer, ret.data, buffer_size);
   return buffer_size;
}

int mp_db_next_i64( int itr, uint64_t* primary ) {
   struct DbNextI64_return ret = DbNextI64(itr);
   assert(ret.r1.len == sizeof(uint64_t));
   memcpy(primary, ret.r1.data, sizeof(*primary));
   return ret.r0;
}

int mp_db_previous_i64( int itr, uint64_t* primary ) {
   struct DbPreviousI64_return ret = DbPreviousI64(itr);
   assert(ret.r1.len == sizeof(uint64_t));
   memcpy(primary, ret.r1.data, sizeof(*primary));
   return ret.r0;
}

int mp_db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return DbFindI64((GoInt64)code, (GoInt64)scope, (GoInt64)table, (GoInt64)id);
}

int mp_db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return DbLowerboundI64((GoInt64)code, (GoInt64)scope, (GoInt64)table, (GoInt64)id);
}

int mp_db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return DbUpperboundI64((GoInt64)code, (GoInt64)scope, (GoInt64)table, (GoInt64)id);
}

int mp_db_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
   return DbEndI64((GoInt64)code, (GoInt64)scope, (GoInt64)table);
}

static struct eosapi s_eosapi;

//mpeoslib.cpp
typedef struct eosapi* (*fn_get_eosapi)();
void set_get_eosapi_func(fn_get_eosapi fn);

struct eosapi* get_eosapi() {
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
   s_eosapi.sha1 = sha1;
   s_eosapi.sha256 = sha256;
   s_eosapi.sha512 = sha512;
   s_eosapi.ripemd160 = ripemd160;

   s_eosapi.string_to_uint64_ = string_to_uint64_;
   s_eosapi.uint64_to_string_ = uint64_to_string_;

   s_eosapi.pack_ = pack_;
   s_eosapi.unpack_ = unpack_;


   s_eosapi.db_store_i64 = db_store_i64;
   s_eosapi.db_update_i64 = db_update_i64;
   s_eosapi.db_remove_i64 = db_remove_i64;
   s_eosapi.db_get_i64 = db_get_i64;
   s_eosapi.db_get_i64_ex = db_get_i64_ex;
   s_eosapi.db_next_i64 = db_next_i64;
   s_eosapi.db_previous_i64 = db_previous_i64;
   s_eosapi.db_find_i64 = db_find_i64;
   s_eosapi.db_lowerbound_i64 = db_lowerbound_i64;
   s_eosapi.db_upperbound_i64 = db_upperbound_i64;
   s_eosapi.db_end_i64 = db_end_i64;
   set_get_eosapi_func(get_eosapi);
}

