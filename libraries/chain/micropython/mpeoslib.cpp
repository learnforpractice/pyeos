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

static inline apply_context& get_apply_ctx() {
   return *micropython_interface::get().current_apply_context;
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

using namespace eosio::chain;
using namespace eosio::chain::contracts;

namespace eosio { namespace micropython {


template<typename ObjectType>
class db_api {
   using KeyType = typename ObjectType::key_type;
   static constexpr int KeyCount = ObjectType::number_of_keys;
   using KeyArrayType = KeyType[KeyCount];
   using ContextMethodType = int(apply_context::*)(const table_id_object&, const account_name&, const KeyType*, const char*, size_t);

   private:
      int call(ContextMethodType method, const scope_name& scope, const name& table, account_name bta, char* keys, size_t keys_len, char* data, size_t data_len) {
         const auto& t_id = context->find_or_create_table(context->receiver, scope, table);
//         FC_ASSERT(data_len >= KeyCount * sizeof(KeyType), "Data is not long enough to contain keys");
//         const KeyType* keys = reinterpret_cast<const KeyType *>((const char *)data);

         FC_ASSERT(keys_len == KeyCount * sizeof(KeyType), "length of keys is incorrect");

//         const char* record_data =  ((const char*)data) + sizeof(KeyArrayType);
//         size_t record_len = data_len - sizeof(KeyArrayType);
         return (context->*(method))(t_id, bta, reinterpret_cast<const KeyType *>((const char *)keys), data, data_len) + sizeof(KeyArrayType);
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

      int store(const scope_name& scope, const name& table, const account_name& bta, char* keys, size_t keys_len, char* data, size_t data_len) {
         auto res = call(&apply_context::store_record<ObjectType>, scope, table, bta, keys, keys_len, data, data_len);
         //ilog("STORE [${scope},${code},${table}] => ${res} :: ${HEX}", ("scope",scope)("code",context.receiver)("table",table)("res",res)("HEX", fc::to_hex(data, data_len)));
         return res;
      }

      int update(const scope_name& scope, const name& table, const account_name& bta, char* keys, size_t keys_len, char* data, size_t data_len) {
         return call(&apply_context::update_record<ObjectType>, scope, table, bta, keys, keys_len, data, data_len);
      }

      int remove(const scope_name& scope, const name& table, const char* keys) {
         const auto& t_id = context->find_or_create_table(context->receiver, scope, table);
         return context->remove_record<ObjectType>(t_id, reinterpret_cast<const KeyType *>((const char *)keys));
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
   		static db_api<keystr_value_object>& get() {
   			static db_api<keystr_value_object>* instance = nullptr;
   			if (!instance) {
   				instance = new db_api<keystr_value_object>(*micropython_interface::get().current_apply_context);
   			} else {
      			instance->context = micropython_interface::get().current_apply_context;
   			}
   			return *instance;
   		}

      int store_str(const scope_name& scope, const name& table, const account_name& bta,
            const char* key, uint32_t key_len, const char* data, size_t data_len) {
         const auto& t_id = context->find_or_create_table(context->receiver, scope, table);
         const KeyType keys(key, key_len);
         const char* record_data =  ((const char*)data);
         size_t record_len = data_len;
         return context->store_record<keystr_value_object>(t_id, bta, &keys, record_data, record_len);
         //return call(&apply_context::store_record<keystr_value_object>, scope, table, bta, key, key_len, data, data_len);
      }

      int update_str(const scope_name& scope,  const name& table, const account_name& bta,
            const char* key, uint32_t key_len, const char* data, size_t data_len) {
         const auto& t_id = context->find_or_create_table(context->receiver, scope, table);
         const KeyType keys((const char*)key, key_len);
         const char* record_data =  ((const char*)data);
         size_t record_len = data_len;
         return context->update_record<keystr_value_object>(t_id, bta, &keys, record_data, record_len);
         //return call(&apply_context::update_record<keystr_value_object>, scope, table, bta, key, key_len, data, data_len);
      }

      int remove_str(const scope_name& scope, const name& table, const char* key, uint32_t key_len) {
         const auto& t_id = context->find_or_create_table(scope, context->receiver, table);
         const KeyArrayType k = {std::string(key, key_len)};
         return context->remove_record<keystr_value_object>(t_id, k);
      }
};


template<typename IndexType, typename Scope>
class db_index_api {
   using KeyType = typename IndexType::value_type::key_type;
   static constexpr int KeyCount = IndexType::value_type::number_of_keys;
   using KeyArrayType = KeyType[KeyCount];
   using ContextMethodType = int(apply_context::*)(const table_id_object&, KeyType*, char*, size_t);


   int call(ContextMethodType method, const account_name& code, const scope_name& scope, const name& table, const char* keys, size_t keys_len, char* data, size_t data_len) {
      auto maybe_t_id = context->find_table(code, scope, table);
      if (maybe_t_id == nullptr) {
         return -1;
      }

      const auto& t_id = *maybe_t_id;
//      FC_ASSERT(data_len >= KeyCount * sizeof(KeyType), "Data is not long enough to contain keys");
//      KeyType* keys = reinterpret_cast<KeyType *>((char *)data);
      FC_ASSERT(keys_len == KeyCount * sizeof(KeyType), "length of keys is incorrect");

//      char* record_data =  ((char*)data) + sizeof(KeyArrayType);
//      size_t record_len = data_len - sizeof(KeyArrayType);

      auto res = (context->*(method))(t_id, reinterpret_cast<KeyType *>((char *)keys), data, data_len);
      if (res != -1) {
         res += sizeof(KeyArrayType);
      }
      return res;
   }

   db_index_api<IndexType, Scope>(apply_context& ctx) : context(&ctx) {}

   public:
   		apply_context*     context;
   		static db_index_api<IndexType, Scope>& get() {
   			static db_index_api<IndexType, Scope>* instance = nullptr;
   			if (!instance) {
   				instance = new db_index_api<IndexType, Scope>(*micropython_interface::get().current_apply_context);
   			} else {
      			instance->context = micropython_interface::get().current_apply_context;
   			}
   			return *instance;
   		}

      int load(const scope_name& scope, const account_name& code, const name& table, const char* keys, size_t keys_len, char* data, size_t data_len) {
         auto res = call(&apply_context::load_record<IndexType, Scope>, scope, code, table, keys, keys_len, data, data_len);
         return res;
      }

      int front(const scope_name& scope, const account_name& code, const name& table, const char* keys, size_t keys_len, char* data, size_t data_len) {
         auto res = call(&apply_context::front_record<IndexType, Scope>, scope, code, table, keys, keys_len, data, data_len);
         return res;
      }

      int back(const scope_name& scope, const account_name& code, const name& table, const char* keys, size_t keys_len, char* data, size_t data_len) {
         auto res = call(&apply_context::back_record<IndexType, Scope>, scope, code, table, keys, keys_len, data, data_len);
         return res;
      }

      int next(const scope_name& scope, const account_name& code, const name& table, const char* keys, size_t keys_len, char* data, size_t data_len) {
         auto res = call(&apply_context::next_record<IndexType, Scope>, scope, code, table, keys, keys_len, data, data_len);
         return res;
      }

      int previous(const scope_name& scope, const account_name& code, const name& table, const char* keys, size_t keys_len, char* data, size_t data_len) {
         auto res = call(&apply_context::previous_record<IndexType, Scope>, scope, code, table, keys, keys_len, data, data_len);
         return res;
      }

      int lower_bound(const scope_name& scope, const account_name& code, const name& table, const char* keys, size_t keys_len, char* data, size_t data_len) {
         auto res = call(&apply_context::lower_bound_record<IndexType, Scope>, scope, code, table, keys, keys_len, data, data_len);
         return res;
      }

      int upper_bound(const scope_name& scope, const account_name& code, const name& table, const char* keys, size_t keys_len, char* data, size_t data_len) {
         auto res = call(&apply_context::upper_bound_record<IndexType, Scope>, scope, code, table, keys, keys_len, data, data_len);
         return res;
      }

};

template<>
class db_index_api<keystr_value_index, by_scope_primary> {
   using KeyType = std::string;
   static constexpr int KeyCount = 1;
   using KeyArrayType = KeyType[KeyCount];
   using ContextMethodType = int(apply_context::*)(const table_id_object&, KeyType*, char*, size_t);


   int call(ContextMethodType method, const scope_name& scope, const account_name& code, const name& table,
         const char*key, uint32_t key_len, char* data, size_t data_len) {
      auto maybe_t_id = context->find_table(scope, context->receiver, table);
      if (maybe_t_id == nullptr) {
         return 0;
      }

      const auto& t_id = *maybe_t_id;
      //FC_ASSERT(data_len >= KeyCount * sizeof(KeyType), "Data is not long enough to contain keys");
      KeyType keys((const char*)key, key_len); // = reinterpret_cast<KeyType *>((char *)data);

      char* record_data =  ((char*)data); // + sizeof(KeyArrayType);
      size_t record_len = data_len; // - sizeof(KeyArrayType);

      return (context->*(method))(t_id, &keys, record_data, record_len); // + sizeof(KeyArrayType);
   }

   db_index_api<keystr_value_index, by_scope_primary>(apply_context& ctx) : context(&ctx) {}

   public:
		apply_context*     context;
		static db_index_api<keystr_value_index, by_scope_primary>& get() {
			static db_index_api<keystr_value_index, by_scope_primary>* instance = nullptr;
			if (!instance) {
				instance = new db_index_api<keystr_value_index, by_scope_primary>(*micropython_interface::get().current_apply_context);
			} else {
   			instance->context = micropython_interface::get().current_apply_context;
			}
			return *instance;
		}

      int load_str(const scope_name& scope, const account_name& code, const name& table, const char* key, size_t key_len, char* data, size_t data_len) {
         auto res = call(&apply_context::load_record<keystr_value_index, by_scope_primary>, scope, code, table, key, key_len, data, data_len);
         return res;
      }

      int front_str(const scope_name& scope, const account_name& code, const name& table, const char* key, size_t key_len, char* data, size_t data_len) {
         return call(&apply_context::front_record<keystr_value_index, by_scope_primary>, scope, code, table, key, key_len, data, data_len);
      }

      int back_str(const scope_name& scope, const account_name& code, const name& table, const char* key, size_t key_len, char* data, size_t data_len) {
         return call(&apply_context::back_record<keystr_value_index, by_scope_primary>, scope, code, table, key, key_len, data, data_len);
      }

      int next_str(const scope_name& scope, const account_name& code, const name& table, const char* key, size_t key_len, char* data, size_t data_len) {
         return call(&apply_context::next_record<keystr_value_index, by_scope_primary>, scope, code, table, key, key_len, data, data_len);
      }

      int previous_str(const scope_name& scope, const account_name& code, const name& table, const char* key, size_t key_len, char* data, size_t data_len) {
         return call(&apply_context::previous_record<keystr_value_index, by_scope_primary>, scope, code, table, key, key_len, data, data_len);
      }

      int lower_bound_str(const scope_name& scope, const account_name& code, const name& table, const char* key, size_t key_len, char* data, size_t data_len) {
         return call(&apply_context::lower_bound_record<keystr_value_index, by_scope_primary>, scope, code, table, key, key_len, data, data_len);
      }

      int upper_bound_str(const scope_name& scope, const account_name& code, const name& table, const char* key, size_t key_len, char* data, size_t data_len) {
         return call(&apply_context::upper_bound_record<keystr_value_index, by_scope_primary>, scope, code, table, key, key_len, data, data_len);
      }
};


}
}

using db_api_key_value_object                                 = eosio::micropython::db_api<key_value_object>;
using db_api_keystr_value_object                              = eosio::micropython::db_api<keystr_value_object>;

using db_api_key128x128_value_object                          = eosio::micropython::db_api<key128x128_value_object>;
using db_api_key64x64_value_object                            = eosio::micropython::db_api<key64x64_value_object>;
using db_api_key64x64x64_value_object                         = eosio::micropython::db_api<key64x64x64_value_object>;

using db_index_api_key_value_index_by_scope_primary           = eosio::micropython::db_index_api<key_value_index,by_scope_primary>;
using db_index_api_keystr_value_index_by_scope_primary        = eosio::micropython::db_index_api<keystr_value_index,by_scope_primary>;

using db_index_api_key128x128_value_index_by_scope_primary    = eosio::micropython::db_index_api<key128x128_value_index,by_scope_primary>;
using db_index_api_key128x128_value_index_by_scope_secondary  = eosio::micropython::db_index_api<key128x128_value_index,by_scope_secondary>;

using db_index_api_key64x64_value_index_by_scope_primary      = eosio::micropython::db_index_api<key64x64_value_index,by_scope_primary>;
using db_index_api_key64x64_value_index_by_scope_secondary    = eosio::micropython::db_index_api<key64x64_value_index,by_scope_secondary>;

using db_index_api_key64x64x64_value_index_by_scope_primary   = eosio::micropython::db_index_api<key64x64x64_value_index,by_scope_primary>;
using db_index_api_key64x64x64_value_index_by_scope_secondary = eosio::micropython::db_index_api<key64x64x64_value_index,by_scope_secondary>;
using db_index_api_key64x64x64_value_index_by_scope_tertiary  = eosio::micropython::db_index_api<key64x64x64_value_index,by_scope_tertiary>;

using namespace eosio::micropython;
using namespace eosio::chain;

extern "C" {


int store_str_(uint64_t scope, uint64_t table,
      const char* key, uint32_t key_len, const char* data, size_t data_len) {
	return db_api<keystr_value_object>::get().store_str(name(scope), name(table), name(0),
	      key, key_len, data, data_len);
}

int update_str_(uint64_t scope, uint64_t table,
      const char* key, uint32_t key_len, const char* data, size_t data_len) {
	return db_api<keystr_value_object>::get().update_str(name(scope), name(table), name(0),
	      key, key_len, data, data_len);
}

int remove_str_(uint64_t scope, uint64_t table, const char* key, uint32_t key_len) {
	return db_api<keystr_value_object>::get().remove_str(name(scope), name(table), key, key_len);
}


using db_str = db_index_api<keystr_value_index, by_scope_primary>;
int load_str_(uint64_t scope, uint64_t code, uint64_t table, char* key, size_t key_len, char* data, size_t data_len) {
	return db_str::get().load_str(name(scope), name(code), name(table), key, key_len, data, data_len);
}

int front_str_(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len) {
	return db_str::get().front_str(name(scope), name(code), name(table), key, key_len, data, data_len);
}

int back_str_(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len) {
	return db_str::get().back_str(name(scope), name(code), name(table), key, key_len, data, data_len);
}

int next_str_(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len) {
	return db_str::get().next_str(name(scope), name(code), name(table), key, key_len, data, data_len);
}

int previous_str_(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len) {
	return db_str::get().previous_str(name(scope), name(code), name(table), key, key_len, data, data_len);
}

int lower_bound_str_(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len) {
	return db_str::get().lower_bound_str(name(scope), name(code), name(table), key, key_len, data, data_len);
}

int upper_bound_str_(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len) {
	return db_str::get().upper_bound_str(name(scope), name(code), name(table), key, key_len, data, data_len);
}


///////////////////////////////////////////////////////////////////////////////////////////////////

#define DB_METHOD_SEQ(API, SUFFIX) \
   int store_##SUFFIX(uint64_t scope, uint64_t table, char* keys, size_t keys_len, char* data, size_t data_len) { \
		return API::get().store(name(scope), name(table), name(0), keys, keys_len, data, data_len); \
	} \
   int update_##SUFFIX(uint64_t scope, uint64_t table, char* keys, size_t keys_len, char* data, size_t data_len) { \
		return API::get().store(name(scope), name(table), name(0), keys, keys_len, data, data_len); \
	} \
   int remove_##SUFFIX(uint64_t scope, uint64_t table, const char* keys) { \
		return API::get().remove(name(scope), name(table), keys); \
	}

#define DB_INDEX_METHOD_SEQ(API, SUFFIX)\
   int load_##SUFFIX(int64_t scope, int64_t code, int64_t table, char* keys, size_t keys_len, char* data, size_t data_len) {\
		return API::get().load(name(scope), name(code), name(table), keys, keys_len, data, data_len); \
	} \
   int front_##SUFFIX(int64_t scope, int64_t code, int64_t table, char* keys, size_t keys_len, char* data, size_t data_len) {\
		return API::get().front(name(scope), name(code), name(table), keys, keys_len, data, data_len); \
	} \
   int previous_##SUFFIX(int64_t scope, int64_t code, int64_t table, char* keys, size_t keys_len, char* data, size_t data_len) {\
		return API::get().previous(name(scope), name(code), name(table), keys, keys_len, data, data_len); \
	} \
   int back_##SUFFIX(int64_t scope, int64_t code, int64_t table, char* keys, size_t keys_len, char* data, size_t data_len) {\
		return API::get().front(name(scope), name(code), name(table), keys, keys_len, data, data_len); \
	} \
   int next_##SUFFIX(int64_t scope, int64_t code, int64_t table, char* keys, size_t keys_len, char* data, size_t data_len) {\
		return API::get().previous(name(scope), name(code), name(table), keys, keys_len, data, data_len); \
	} \
	int lower_bound_##SUFFIX(int64_t scope, int64_t code, int64_t table, char* keys, size_t keys_len, char* data, size_t data_len) {\
		return API::get().lower_bound(name(scope), name(code), name(table), keys, keys_len, data, data_len); \
	} \
   int upper_bound_##SUFFIX(int64_t scope, int64_t code, int64_t table, char* keys, size_t keys_len, char* data, size_t data_len) {\
		return API::get().upper_bound(name(scope), name(code), name(table), keys, keys_len, data, data_len); \
	}

DB_METHOD_SEQ(db_api_key_value_object, i64)
DB_METHOD_SEQ(db_api_key128x128_value_object, i128i128)
DB_METHOD_SEQ(db_api_key64x64_value_object, i64i64)
DB_METHOD_SEQ(db_api_key64x64x64_value_object, i64i64i64)



DB_INDEX_METHOD_SEQ(db_index_api_key_value_index_by_scope_primary, i64)

DB_INDEX_METHOD_SEQ(db_index_api_key128x128_value_index_by_scope_primary, primary_i128i128)
DB_INDEX_METHOD_SEQ(db_index_api_key128x128_value_index_by_scope_secondary, secondary_i128i128)

DB_INDEX_METHOD_SEQ(db_index_api_key64x64_value_index_by_scope_primary, primary_i64i64)
DB_INDEX_METHOD_SEQ(db_index_api_key64x64_value_index_by_scope_secondary, secondary_i64i64)

DB_INDEX_METHOD_SEQ(db_index_api_key64x64x64_value_index_by_scope_primary, primary_i64i64i64)
DB_INDEX_METHOD_SEQ(db_index_api_key64x64x64_value_index_by_scope_secondary, secondary_i64i64i64)
DB_INDEX_METHOD_SEQ(db_index_api_key64x64x64_value_index_by_scope_tertiary, tertiary_i64i64i64)

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

uint32_t now_() {
   auto& ctrl = get_apply_ctx().controller;
   return ctrl.head_block_time().sec_since_epoch();
}

}



