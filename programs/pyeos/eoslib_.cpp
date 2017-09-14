#include "eoslib_.hpp"
#include <eos/chain/python_interface.hpp>

using namespace eos;
using namespace eos::chain;

typedef long long            int64_t;
typedef unsigned long long   uint64_t;

typedef uint64_t TableName;

uint64_t string_to_name_( const char* str ) {
   return Name(str).value;
}

static inline apply_context& get_apply_ctx() {
   return *python_interface::get().current_apply_context;
}

static inline apply_context& get_validate_ctx() {
   return *python_interface::get().current_validate_context;
}

void  requireAuth_( uint64_t account ){
   get_validate_ctx().require_authorization( Name(account) );
}


uint64_t currentCode_() {
   return get_validate_ctx().code.value;
}

int readMessage_( string& buffer ){
   buffer = string(get_validate_ctx().msg.data.data(), get_validate_ctx().msg.data.size() );
   return buffer.size();
}

void requireScope_( uint64_t account ) {
   get_validate_ctx().require_scope( Name(account) );
}

void requireNotice_( uint64_t account ) {
   get_validate_ctx().require_recipient( Name(account) );
}

int store_i128i128(AccountName scope, TableName table, char* data, uint32_t len){
//   key128x128_value_index
   apply_context& ctx = get_apply_ctx();
   key128x128_value_index::value_type::key_type* keys;
/*
   key_value_object
   key128x128_value_object
   key64x64x64_value_object
*/
   return ctx.store_record<key128x128_value_index::value_type>(Name(scope), Name(ctx.code.value),Name(table), keys, data, len);
}

int32_t update_( Name scope, Name code, Name table, void *keys, int key_type,char* value, uint32_t valuelen ) {
   return 0;
}

int32_t remove_( Name scope, Name code, Name table, void *keys, int key_type, char* value, uint32_t valuelen ) {
   return 0;
}

int32_t load_( Name scope, Name code, Name table, void *keys,int key_type, char* value, uint32_t valuelen ) {
   return 0;
}

int32_t front_( Name scope, Name code, Name table, void *keys,int key_type, char* value, uint32_t valuelen ) {
   return 0;
}

int32_t back_( Name scope, Name code, Name table, void *keys,int key_type, char* value, uint32_t valuelen ) {
   return 0;
}


int32_t next_( Name scope, Name code, Name table, void *keys,int key_type, char* value, uint32_t valuelen ) {
   return 0;
}

int32_t previous_( Name scope, Name code, Name table, void *keys,int key_type, char* value, uint32_t valuelen ) {
   return 0;
}

int32_t lower_bound_( Name scope, Name code, Name table, void *keys,int key_type, char* value, uint32_t valuelen ) {
   return 0;
}

int32_t upper_bound_( Name scope, Name code, Name table, void *keys,int key_type, char* value, uint32_t valuelen ) {
   return 0;
}

