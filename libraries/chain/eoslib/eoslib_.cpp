#include <eos/chain/chain_controller.hpp>
#include <eos/chain/python_interface.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/raw.hpp>

#include "eoslib_.hpp"


using namespace eosio;
using namespace eosio::chain;

typedef long long int64_t;
typedef unsigned long long uint64_t;

typedef uint64_t TableName;

uint64_t string_to_uint64_(string str) {
   try {
      return name(str).value;
   } catch (fc::assert_exception& e) {
      elog(e.to_detail_string());
   } catch (fc::exception& e) {
      elog(e.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   return 0;
}

string uint64_to_string_(uint64_t n) { return name(n).to_string(); }

void pack_(string& raw, string& out) {
   std::vector<char> o = fc::raw::pack<string>(raw);
   out = string(o.begin(), o.end());
}

void unpack_(string& raw, string& out) {
   std::vector<char> v(raw.begin(), raw.end());
   out = fc::raw::unpack<string>(v);
}

static inline apply_context& get_apply_ctx() {
   return *python_interface::get().current_apply_context;
}

static inline apply_context& get_validate_ctx() {
   return *python_interface::get().current_validate_context;
}

void new_apply_context() {

}

void requireAuth_(uint64_t account) {
   get_validate_ctx().require_authorization(name(account));
}

uint32_t now_() {
   auto& ctrl = get_validate_ctx().controller;
   return ctrl.head_block_time().sec_since_epoch();
}

uint64_t currentCode_() { return get_validate_ctx().code.value; }

int readMessage_(string& buffer) {
   buffer = string(get_validate_ctx().msg.data.data(),
                   get_validate_ctx().msg.data.size());
   return buffer.size();
}

void requireScope_(uint64_t account) {
   get_validate_ctx().require_scope(name(account));
}

void requireNotice_(uint64_t account) {
   get_validate_ctx().require_recipient(name(account));
}

#define RETURN_WRITE_RECORD(NAME, VALUE_OBJECT)       \
   return ctx.NAME##_record<VALUE_OBJECT>(             \
       scope, ctx.code, table, \
       (VALUE_OBJECT::key_type*)keys, value, valuelen)

#define RETURN_READ_RECORD(NAME)                                        \
   FC_ASSERT(key_value_index::value_type::number_of_keys > scope_index, \
             "scope index out off bound");                              \
   return ctx.NAME##_record<key_value_index, by_scope_primary>(         \
       scope, code, table,                            \
       (key_value_index::value_type::key_type*)keys, value, valuelen);

#define RETURN_READ_RECORD_KEY128x128(NAME)                                    \
   FC_ASSERT(key128x128_value_index::value_type::number_of_keys > scope_index, \
             "scope index out off bound");                                     \
   if (scope_index == 0) {                                                     \
      return ctx.NAME##_record<key128x128_value_index, by_scope_primary>(      \
          scope, code, table,                                \
          (key128x128_value_index::value_type::key_type*)keys, value,          \
          valuelen);                                                           \
   } else if (scope_index == 1) {                                              \
      return ctx.NAME##_record<key128x128_value_index, by_scope_secondary>(    \
          scope, code, table,                                \
          (key128x128_value_index::value_type::key_type*)keys, value,          \
          valuelen);                                                           \
   }

#define RETURN_READ_RECORD_KEY64x64x64(NAME)                                 \
   FC_ASSERT(                                                                \
       key64x64x64_value_index::value_type::number_of_keys > scope_index,    \
       "scope index out off bound");                                         \
   if (scope_index == 0) {                                                   \
      return ctx.NAME##_record<key64x64x64_value_index, by_scope_primary>(   \
          scope, code, table,                              \
          (key64x64x64_value_index::value_type::key_type*)keys, value,       \
          valuelen);                                                         \
   } else if (scope_index == 1) {                                            \
      return ctx.NAME##_record<key64x64x64_value_index, by_scope_secondary>( \
          scope, code, table,                              \
          (key64x64x64_value_index::value_type::key_type*)keys, value,       \
          valuelen);                                                         \
   } else if (scope_index == 2) {                                            \
      return ctx.NAME##_record<key64x64x64_value_index, by_scope_tertiary>(  \
          scope, code, table,                              \
          (key64x64x64_value_index::value_type::key_type*)keys, value,       \
          valuelen);                                                         \
   }

#define RETURN_WRITE_RECORD_STR(OPERATION) \
{ \
   std::string str_key((char*)keys, keylen); \
   return ctx.OPERATION##_record<keystr_value_object>( scope, ctx.code, table, &str_key, value, valuelen ); \
}

#define RETURN_READ_RECORD_STR(OPERATION) \
{ \
   std::string str_key((char*)keys, keylen); \
   return ctx.OPERATION##_record<keystr_value_index, by_scope_primary>( scope, code, table, &str_key, value, valuelen); \
}

#define DEFINE_WRITE_FUNCTION(OPERATION) \
int32_t OPERATION##_(name scope, name table, void* keys, int keylen, int key_type, \
                char* value, uint32_t valuelen) { \
   apply_context& ctx = get_apply_ctx(); \
   if (key_type == 0) { \
      RETURN_WRITE_RECORD(OPERATION, key_value_object); \
   } else if (key_type == 1) { \
      RETURN_WRITE_RECORD(OPERATION, key128x128_value_object); \
   } else if (key_type == 2) { \
      RETURN_WRITE_RECORD(OPERATION, key64x64x64_value_object); \
   } else if (key_type == 3) { \
      RETURN_WRITE_RECORD_STR(OPERATION); \
   } \
   return 0; \
}

#define DEFINE_READ_FUNCTION(OPERATION) \
      int32_t OPERATION##_(name scope, name code, name table, void* keys, int keylen, int key_type, \
                    int scope_index, char* value, uint32_t valuelen) { \
   FC_ASSERT(scope_index >= 0, "scope index must be >= 0"); \
   apply_context& ctx = get_apply_ctx(); \
   if (key_type == 0) { \
      RETURN_READ_RECORD(OPERATION); \
   } else if (key_type == 1) { \
      RETURN_READ_RECORD_KEY128x128(OPERATION); \
   } else if (key_type == 2) { \
      RETURN_READ_RECORD_KEY64x64x64(OPERATION); \
   } else if (key_type == 3) { \
      RETURN_READ_RECORD_STR(OPERATION); \
   } \
   return 0; \
}


DEFINE_WRITE_FUNCTION(store)
DEFINE_WRITE_FUNCTION(update)
DEFINE_WRITE_FUNCTION(remove)

DEFINE_READ_FUNCTION(load)
DEFINE_READ_FUNCTION(front)
DEFINE_READ_FUNCTION(back)
DEFINE_READ_FUNCTION(next)
DEFINE_READ_FUNCTION(previous)
DEFINE_READ_FUNCTION(lower_bound)
DEFINE_READ_FUNCTION(upper_bound)


