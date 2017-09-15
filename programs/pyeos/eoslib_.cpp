#include "eoslib_.hpp"
#include <fc/exception/exception.hpp>
#include <eos/chain/python_interface.hpp>

using namespace eos;
using namespace eos::chain;

typedef long long            int64_t;
typedef unsigned long long   uint64_t;

typedef uint64_t TableName;

uint64_t string_to_uint64_( string str ) {
   return Name(str).value;
}

string uint64_to_string_( uint64_t n) {
   return Name(n).toString();
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

#define UPDATE_RECORD(NAME,VALUE_OBJECT) \
   ctx.NAME##_record<VALUE_OBJECT>(Name(scope), Name(ctx.code.value),Name(table), (VALUE_OBJECT::key_type*)keys, value, valuelen)

#define READ_RECORD(NAME,VALUE_INDEX) \
      FC_ASSERT(key_value_index::value_type::number_of_keys > scope_index,"scope index out off bound"); \
      keys = (void*)(((VALUE_INDEX::value_type::key_type*)keys) + scope_index); \
      return ctx.NAME##_record<VALUE_INDEX,by_scope_primary>(Name(scope), Name(code),Name(table), (VALUE_INDEX::value_type::key_type*)keys, value, valuelen)

//      FC_ASSERT(VALUE_INDEX::value_type::number_of_keys<scope_index,"scope index out off bound");


int32_t store_(Name scope, Name code, Name table, void *keys, int key_type, char* value, uint32_t valuelen){
//   key128x128_value_index
   apply_context& ctx = get_apply_ctx();
//   return ctx.store_record<key_value_object>(Name(scope), Name(ctx.code.value),Name(table), (key_value_object::key_type*)keys, data, valuelen);

   if (key_type == 0) {
      UPDATE_RECORD(store,key_value_object);
   } else if (key_type == 1) {
      return UPDATE_RECORD(store,key128x128_value_object);
   } else if (key_type == 2) {
      return UPDATE_RECORD(store,key64x64x64_value_object);
   }
   return 0;
/*
   key_value_object
   key128x128_value_index
   key64x64x64_value_index
*/
}

int32_t update_( Name scope, Name code, Name table, void *keys, int key_type,char* value, uint32_t valuelen ) {
   apply_context& ctx = get_apply_ctx();
   if (key_type == 0) {
      return UPDATE_RECORD(update,key_value_object);
   } else if (key_type == 1) {
      return UPDATE_RECORD(update,key128x128_value_object);
   } else if (key_type == 2) {
      return UPDATE_RECORD(update,key64x64x64_value_object);
   }

   return 0;
}

int32_t remove_( Name scope, Name code, Name table, void *keys, int key_type, char* value, uint32_t valuelen ) {
   apply_context& ctx = get_apply_ctx();
   if (key_type == 0) {
      return UPDATE_RECORD(remove,key_value_object);
   } else if (key_type == 1) {
      return UPDATE_RECORD(remove,key128x128_value_object);
   } else if (key_type == 2) {
      return UPDATE_RECORD(remove,key64x64x64_value_object);
   }
   return 0;
}

int32_t load_( Name scope, Name code, Name table, void *keys,int key_type, int scope_index, char* value, uint32_t valuelen ) {

   FC_ASSERT(scope_index >= 0,"scope index must be >= 0");

   apply_context& ctx = get_apply_ctx();

   if (key_type == 0) {
      READ_RECORD(load,key_value_index);
   } else if (key_type == 1) {
      READ_RECORD(load,key128x128_value_index);
   } else if (key_type == 2) {
      READ_RECORD(load,key64x64x64_value_index);
   }
   return 0;
}

int32_t front_( Name scope, Name code, Name table, void *keys,int key_type,int scope_index, char* value, uint32_t valuelen ) {

   FC_ASSERT(scope_index >= 0,"scope index must be >= 0");

   apply_context& ctx = get_apply_ctx();

   if (key_type == 0) {
      READ_RECORD(front,key_value_index);
   } else if (key_type == 1) {
      READ_RECORD(front,key128x128_value_index);
   } else if (key_type == 2) {
      READ_RECORD(front,key64x64x64_value_index);
   }
   return 0;
}

int32_t back_( Name scope, Name code, Name table, void *keys,int key_type, int scope_index, char* value, uint32_t valuelen ) {

   FC_ASSERT(scope_index >= 0,"scope index must be >= 0");

   apply_context& ctx = get_apply_ctx();

   if (key_type == 0) {
      READ_RECORD(back,key_value_index);
   } else if (key_type == 1) {
      READ_RECORD(back,key128x128_value_index);
   } else if (key_type == 2) {
      READ_RECORD(back,key64x64x64_value_index);
   }
   return 0;
}


int32_t next_( Name scope, Name code, Name table, void *keys,int key_type, int scope_index, char* value, uint32_t valuelen ) {

   FC_ASSERT(scope_index >= 0,"scope index must be >= 0");

   apply_context& ctx = get_apply_ctx();

   if (key_type == 0) {
      READ_RECORD(next,key_value_index);
   } else if (key_type == 1) {
      READ_RECORD(next,key128x128_value_index);
   } else if (key_type == 2) {
      READ_RECORD(next,key64x64x64_value_index);
   }
   return 0;
}

int32_t previous_( Name scope, Name code, Name table, void *keys,int key_type, int scope_index, char* value, uint32_t valuelen ) {

   FC_ASSERT(scope_index >= 0,"scope index must be >= 0");
   apply_context& ctx = get_apply_ctx();
   if (key_type == 0) {
      READ_RECORD(previous,key_value_index);
   } else if (key_type == 1) {
      READ_RECORD(previous,key128x128_value_index);
   } else if (key_type == 2) {
      READ_RECORD(previous,key64x64x64_value_index);
   }
   return 0;
}

int32_t lower_bound_( Name scope, Name code, Name table, void *keys,int key_type, int scope_index, char* value, uint32_t valuelen ) {
   FC_ASSERT(scope_index >= 0,"scope index must be >= 0");
   apply_context& ctx = get_apply_ctx();
   if (key_type == 0) {
      READ_RECORD(lower_bound,key_value_index);
   } else if (key_type == 1) {
      READ_RECORD(lower_bound,key128x128_value_index);
   } else if (key_type == 2) {
      READ_RECORD(lower_bound,key64x64x64_value_index);
   }
   return 0;
}

int32_t upper_bound_( Name scope, Name code, Name table, void *keys,int key_type, int scope_index, char* value, uint32_t valuelen ) {
   apply_context& ctx = get_apply_ctx();

   FC_ASSERT(scope_index >= 0,"scope index must be >= 0");

   if (key_type == 0) {
      READ_RECORD(upper_bound,key_value_index);
   } else if (key_type == 1) {
      READ_RECORD(upper_bound,key128x128_value_index);
   } else if (key_type == 2) {
      READ_RECORD(upper_bound,key64x64x64_value_index);
   }
   return 0;
}

