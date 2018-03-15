#include "mpeoslib.h"

#include <eosio/chain/chain_controller.hpp>
#include <eosio/chain/micropython_interface.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/raw.hpp>



#include <eosio/chain/wasm_interface.hpp>
#include <eosio/chain/apply_context.hpp>
#include <eosio/chain/chain_controller.hpp>
#include <eosio/chain/producer_schedule.hpp>
#include <eosio/chain/asset.hpp>
#include <eosio/chain/exceptions.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <eosio/chain/wasm_interface_private.hpp>
#include <eosio/chain/wasm_eosio_constraints.hpp>
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
	apply_context *ctx = nullptr;
	return *ctx;
//   return *micropython_interface::get().current_apply_context;
}

#if 0
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
   return mp_obj_new_bytes((unsigned char *)bs.data(), bs.size());
}

void require_scope_(uint64_t account) {
   get_validate_ctx().require_scope(name(account));
}

void require_notice_(uint64_t account) {
   get_validate_ctx().require_recipient(name(account));
}
#endif

#if 0
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

#endif

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

namespace eosio { namespace chain {
using namespace contracts;


template<typename ObjectType>
class db_api {
   using KeyType = typename ObjectType::key_type;
   static constexpr int KeyCount = ObjectType::number_of_keys;
   using KeyArrayType = KeyType[KeyCount];
   using ContextMethodType = int(apply_context::*)(const table_id_object&, const account_name&, const KeyType*, const char*, size_t);

   private:
      int call(ContextMethodType method, const scope_name& scope, const name& table, account_name bta, array_ptr<const char> data, size_t data_len) {
         const auto& t_id = context->find_or_create_table(context->receiver, scope, table);
         FC_ASSERT(data_len >= KeyCount * sizeof(KeyType), "Data is not long enough to contain keys");
         const KeyType* keys = reinterpret_cast<const KeyType *>((const char *)data);

         const char* record_data =  ((const char*)data) + sizeof(KeyArrayType);
         size_t record_len = data_len - sizeof(KeyArrayType);
         return (context->*(method))(t_id, bta, keys, record_data, record_len) + sizeof(KeyArrayType);
      }

      db_api<ObjectType>(apply_context& ctx) : context(&ctx) {}

   public:
      apply_context*     context;
      static db_api<ObjectType>& get() {
			static db_api<ObjectType>* instance = nullptr;
			if (!instance) {
				instance = new db_api<ObjectType>(*micropython_interface::get().current_apply_context);
			} else {
				instance->context = micropython_interface::get().current_apply_context;
			}
			return *instance;
		}

      int store(const scope_name& scope, const name& table, const account_name& bta, array_ptr<const char> data, size_t data_len) {
         auto res = call(&apply_context::store_record<ObjectType>, scope, table, bta, data, data_len);
         //ilog("STORE [${scope},${code},${table}] => ${res} :: ${HEX}", ("scope",scope)("code",context.receiver)("table",table)("res",res)("HEX", fc::to_hex(data, data_len)));
         return res;
      }

      int update(const scope_name& scope, const name& table, const account_name& bta, array_ptr<const char> data, size_t data_len) {
         return call(&apply_context::update_record<ObjectType>, scope, table, bta, data, data_len);
      }

      int remove(const scope_name& scope, const name& table, const KeyArrayType &keys) {
         const auto& t_id = context->find_or_create_table(context->receiver, scope, table);
         return context->remove_record<ObjectType>(t_id, keys);
      }
};

template<>
class db_api<keystr_value_object> {
   using KeyType = std::string;
   static constexpr int KeyCount = 1;
   using KeyArrayType = KeyType[KeyCount];
   using ContextMethodType = int(apply_context::*)(const table_id_object&, const KeyType*, const char*, size_t);


/* TODO something weird is going on here, will maybe fix before DB changes or this might get
 * totally changed anyway
   private:
      int call(ContextMethodType method, const scope_name& scope, const name& table, account_name bta,
            null_terminated_ptr key, size_t key_len, array_ptr<const char> data, size_t data_len) {
         const auto& t_id = context.find_or_create_table(context.receiver, scope, table);
         const KeyType keys((const char*)key.value, key_len);

         const char* record_data =  ((const char*)data);
         size_t record_len = data_len;
         return (context.*(method))(t_id, bta, &keys, record_data, record_len);
      }
*/
	db_api<keystr_value_object>(apply_context& ctx) : context(&ctx) {}

   public:
   		apply_context*     context;
   		static db_api<keystr_value_object>& get_instance() {
   			static db_api<keystr_value_object>* instance = nullptr;
   			if (!instance) {
   				instance = new db_api<keystr_value_object>(*micropython_interface::get().current_apply_context);
   			} else {
      			instance->context = micropython_interface::get().current_apply_context;
   			}
   			return *instance;
   		}

      int store_str(const scope_name& scope, const name& table, const account_name& bta,
            null_terminated_ptr key, uint32_t key_len, array_ptr<const char> data, size_t data_len) {
         const auto& t_id = context->find_or_create_table(context->receiver, scope, table);
         const KeyType keys(key.value, key_len);
         const char* record_data =  ((const char*)data);
         size_t record_len = data_len;
         return context->store_record<keystr_value_object>(t_id, bta, &keys, record_data, record_len);
         //return call(&apply_context::store_record<keystr_value_object>, scope, table, bta, key, key_len, data, data_len);
      }

      int update_str(const scope_name& scope,  const name& table, const account_name& bta,
            null_terminated_ptr key, uint32_t key_len, array_ptr<const char> data, size_t data_len) {
         const auto& t_id = context->find_or_create_table(context->receiver, scope, table);
         const KeyType keys((const char*)key, key_len);
         const char* record_data =  ((const char*)data);
         size_t record_len = data_len;
         return context->update_record<keystr_value_object>(t_id, bta, &keys, record_data, record_len);
         //return call(&apply_context::update_record<keystr_value_object>, scope, table, bta, key, key_len, data, data_len);
      }

      int remove_str(const scope_name& scope, const name& table, array_ptr<const char> &key, uint32_t key_len) {
         const auto& t_id = context->find_or_create_table(scope, context->receiver, table);
         const KeyArrayType k = {std::string(key, key_len)};
         return context->remove_record<keystr_value_object>(t_id, k);
      }
};

}
}

