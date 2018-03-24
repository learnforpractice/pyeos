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

}

using namespace eosio::chain;
using namespace eosio::chain::contracts;

namespace eosio { namespace micropython {


static inline apply_context& get_apply_ctx() {
   return *get_current_context();
}

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
				instance = new db_api<ObjectType>(*get_current_context());
			} else {
				instance->context = get_current_context();
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
   				instance = new db_api<keystr_value_object>(*get_current_context());
   			} else {
      			instance->context = get_current_context();
   			}
   			return *instance;
   		}

   		account_name code_account() {
   			if ( context->act.account == eosio::chain::contracts::setcode::get_account() ) {
   				if ( context->act.name == eosio::chain::contracts::setcode::get_name() ) {
   					auto  act = context->act.data_as<setcode>();
   					return act.account;
   				}
   			}
   			return context->receiver;
   		}

      int store_str(const scope_name& scope, const name& table, const account_name& bta,
            const char* key, uint32_t key_len, const char* data, size_t data_len) {
         const auto& t_id = context->find_or_create_table(code_account(), scope, table);
         const KeyType keys(key, key_len);
         const char* record_data =  ((const char*)data);
         size_t record_len = data_len;
         return context->store_record<keystr_value_object>(t_id, bta, &keys, record_data, record_len);
         //return call(&apply_context::store_record<keystr_value_object>, scope, table, bta, key, key_len, data, data_len);
      }

      int update_str(const scope_name& scope,  const name& table, const account_name& bta,
            const char* key, uint32_t key_len, const char* data, size_t data_len) {
         const auto& t_id = context->find_or_create_table(code_account(), scope, table);
         const KeyType keys((const char*)key, key_len);
         const char* record_data =  ((const char*)data);
         size_t record_len = data_len;
         return context->update_record<keystr_value_object>(t_id, bta, &keys, record_data, record_len);
         //return call(&apply_context::update_record<keystr_value_object>, scope, table, bta, key, key_len, data, data_len);
      }

      int remove_str(const scope_name& scope, const name& table, const char* key, uint32_t key_len) {
         const auto& t_id = context->find_or_create_table(scope, code_account(), table);
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
   				instance = new db_index_api<IndexType, Scope>(*get_current_context());
   			} else {
      			instance->context = get_current_context();
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

#if 0
	account_name code_account() {
		if ( context->act.account == eosio::chain::contracts::setcode::get_account() ) {
			if ( context->act.name == eosio::chain::contracts::setcode::get_name() ) {
				auto  act = context->act.data_as<setcode>();
				return act.account;
			}
		}
		return context->receiver;
	}
#endif

   int call(ContextMethodType method, const scope_name& scope, const account_name& code, const name& table,
         const char*key, uint32_t key_len, char* data, size_t data_len) {
      auto maybe_t_id = context->find_table(scope, code, table);
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
				instance = new db_index_api<keystr_value_index, by_scope_primary>(*get_current_context());
			} else {
				instance->context = get_current_context();
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


int store_str(uint64_t scope, uint64_t table,
      const char* key, uint32_t key_len, const char* data, size_t data_len) {
	return db_api<keystr_value_object>::get().store_str(name(scope), name(table), name(0),
	      key, key_len, data, data_len);
}

int update_str(uint64_t scope, uint64_t table,
      const char* key, uint32_t key_len, const char* data, size_t data_len) {
	return db_api<keystr_value_object>::get().update_str(name(scope), name(table), name(0),
	      key, key_len, data, data_len);
}

int remove_str(uint64_t scope, uint64_t table, const char* key, uint32_t key_len) {
	return db_api<keystr_value_object>::get().remove_str(name(scope), name(table), key, key_len);
}


using db_str = db_index_api<keystr_value_index, by_scope_primary>;
int load_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len) {
	return db_str::get().load_str(name(scope), name(code), name(table), key, key_len, data, data_len);
}

int front_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len) {
	return db_str::get().front_str(name(scope), name(code), name(table), key, key_len, data, data_len);
}

int back_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len) {
	return db_str::get().back_str(name(scope), name(code), name(table), key, key_len, data, data_len);
}

int next_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len) {
	return db_str::get().next_str(name(scope), name(code), name(table), key, key_len, data, data_len);
}

int previous_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len) {
	return db_str::get().previous_str(name(scope), name(code), name(table), key, key_len, data, data_len);
}

int lower_bound_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len) {
	return db_str::get().lower_bound_str(name(scope), name(code), name(table), key, key_len, data, data_len);
}

int upper_bound_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len) {
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

uint32_t now() {
   auto& ctrl = get_apply_ctx().controller;
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

	return mp_obj_new_str(pub, pubds.tellp());
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
	return mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

mp_obj_t sha256(const char* data, size_t datalen) {
	string str_hash = fc::sha256::hash( data, datalen ).str();
	return mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

mp_obj_t sha512(const char* data, size_t datalen) {
	string str_hash = fc::sha512::hash( data, datalen ).str();
	return mp_obj_new_str(str_hash.c_str(), str_hash.size());
}

mp_obj_t ripemd160(const char* data, size_t datalen) {
	string str_hash = fc::ripemd160::hash( data, datalen ).str();
	return mp_obj_new_str(str_hash.c_str(), str_hash.size());
}


}

