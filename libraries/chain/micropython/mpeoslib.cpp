#include "mpeoslib.h"

#include <eos/chain/chain_controller.hpp>
#include <eos/chain/micropython_interface.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/raw.hpp>
#include <eos/chain/balance_object.hpp>
#include <eos/chain/staked_balance_objects.hpp>


using namespace eosio;
using namespace eosio::chain;

extern "C" {
//typedef long long int64_t;
//typedef unsigned long long uint64_t;

typedef uint64_t TableName;

uint64_t string_to_uint64_(const char* str) {
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

mp_obj_t uint64_to_string_(uint64_t n) {
   string s = name(n).to_string();
   ilog("n: ${n}",("n", s.size()));
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

static inline apply_context& get_apply_ctx() {
   return *micropython_interface::get().current_apply_context;
}

static inline apply_context& get_validate_ctx() {
   return *micropython_interface::get().current_validate_context;
}

void new_apply_context() {

}

void require_auth_(uint64_t account) {
   get_validate_ctx().require_authorization(name(account));
}

uint32_t now_() {
   auto& ctrl = get_validate_ctx().controller;
   return ctrl.head_block_time().sec_since_epoch();
}

uint64_t current_code_() { return get_validate_ctx().code.value; }

mp_obj_t read_message_() {
   auto& bs = get_validate_ctx().msg.data;
   return mp_obj_new_str(bs.data(), bs.size());
}

void require_scope_(uint64_t account) {
   get_validate_ctx().require_scope(name(account));
}

void require_notice_(uint64_t account) {
   get_validate_ctx().require_recipient(name(account));
}

#define RETURN_WRITE_RECORD(NAME, VALUE_OBJECT)       \
   return ctx.NAME##_record<VALUE_OBJECT>(             \
       name(scope), name(ctx.code), name(table), \
       (VALUE_OBJECT::key_type*)keys, value, valuelen)

#define RETURN_READ_RECORD(NAME)                                        \
   FC_ASSERT(key_value_index::value_type::number_of_keys > scope_index, \
             "scope index out off bound");                              \
   return ctx.NAME##_record<key_value_index, by_scope_primary>(         \
       name(scope), name(code), name(table),                            \
       (key_value_index::value_type::key_type*)keys, value, valuelen);

#define RETURN_READ_RECORD_KEY128x128(NAME)                                    \
   FC_ASSERT(key128x128_value_index::value_type::number_of_keys > scope_index, \
             "scope index out off bound");                                     \
   if (scope_index == 0) {                                                     \
      return ctx.NAME##_record<key128x128_value_index, by_scope_primary>(      \
          name(scope), name(code), name(table),                                \
          (key128x128_value_index::value_type::key_type*)keys, value,          \
          valuelen);                                                           \
   } else if (scope_index == 1) {                                              \
      return ctx.NAME##_record<key128x128_value_index, by_scope_secondary>(    \
          name(scope), name(code), name(table),                                \
          (key128x128_value_index::value_type::key_type*)keys, value,          \
          valuelen);                                                           \
   }

#define RETURN_READ_RECORD_KEY64x64x64(NAME)                                 \
   FC_ASSERT(                                                                \
       key64x64x64_value_index::value_type::number_of_keys > scope_index,    \
       "scope index out off bound");                                         \
   if (scope_index == 0) {                                                   \
      return ctx.NAME##_record<key64x64x64_value_index, by_scope_primary>(   \
          name(scope), name(code), name(table),                              \
          (key64x64x64_value_index::value_type::key_type*)keys, value,       \
          valuelen);                                                         \
   } else if (scope_index == 1) {                                            \
      return ctx.NAME##_record<key64x64x64_value_index, by_scope_secondary>( \
          name(scope), name(code), name(table),                              \
          (key64x64x64_value_index::value_type::key_type*)keys, value,       \
          valuelen);                                                         \
   } else if (scope_index == 2) {                                            \
      return ctx.NAME##_record<key64x64x64_value_index, by_scope_tertiary>(  \
          name(scope), name(code), name(table),                              \
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
   return ctx.OPERATION##_record<keystr_value_index, by_scope_primary>( name(scope), name(code), name(table), &str_key, value, valuelen); \
}

#define DEFINE_WRITE_FUNCTION(OPERATION) \
int32_t OPERATION##_(Name scope, Name table, void* keys, int keylen, int key_type, \
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
      int32_t OPERATION##_(Name scope, Name code, Name table, void* keys, int keylen, int key_type, \
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

mp_obj_t get_account_balance_(Name account) {

   get_validate_ctx().require_scope(account);

   auto& db = get_validate_ctx().db;
   auto* balance        = db.find< balance_object,by_owner_name >( account );
   auto* sbo = db.find< staked_balance_object,by_owner_name >( account );

   if (balance == nullptr || sbo == nullptr)
     return 0;

   mp_obj_tuple_t *tuple = (mp_obj_tuple_t *)mp_obj_new_tuple(5, NULL);

   tuple->items[0] = mp_obj_new_int(balance->balance);
   tuple->items[1] = mp_obj_new_int(sbo->staked_balance);
   tuple->items[2] = mp_obj_new_int(sbo->unstaking_balance);
   tuple->items[3] = mp_obj_new_int(sbo->last_unstaking_time.sec_since_epoch());

   return MP_OBJ_FROM_PTR(tuple);
}

}

#if 0
uint64_t get_active_producers_() {
   account_name _name;
   get_validate_ctx().get_active_producers(&_name, sizeof(_name));
   return _name.value;
}

void  sha256_(string& data, string& hash) {
   auto v  = fc::sha256::hash(data);
   hash = v.str();
}
#endif
