#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "_cgo_export.h"
#include "mpeoslib.h"

int call_onApply(uint64_t receiver, uint64_t code, uint64_t act)
{
// int onApply(uint64_t, uint64_t, uint64_t);
   return onApply(receiver, code, act);
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
int register_eosapi(struct eosapi* eosapi);
struct mpapi* get_mpapi_();

void mp_init_eosapi() {
   static bool _init = false;
   if (_init) {
      return;
   }
   _init = true;

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
   register_eosapi(&s_eosapi);
}


