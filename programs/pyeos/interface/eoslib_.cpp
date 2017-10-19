#include "eoslib_.hpp"
#include <fc/exception/exception.hpp>
#include <eos/chain/python_interface.hpp>
#include <eos/chain/chain_controller.hpp>
#include <fc/io/raw.hpp>
using namespace eos;
using namespace eos::chain;

typedef long long int64_t;
typedef unsigned long long uint64_t;

typedef uint64_t TableName;

uint64_t string_to_uint64_(string str) {
   try {
      return Name(str).value;
   } catch (fc::assert_exception& e) {
      elog(e.to_detail_string());
   } catch (fc::exception& e) {
      elog(e.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   return 0;
}

string uint64_to_string_(uint64_t n) {
   return Name(n).toString();
}

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

void requireAuth_(uint64_t account) {
   get_validate_ctx().require_authorization(Name(account));
}

uint32_t now_() {
   return get_validate_ctx().controller.head_block_time().sec_since_epoch();
}

uint64_t currentCode_() {
   return get_validate_ctx().code.value;
}

int readMessage_(string& buffer) {
   buffer = string(get_validate_ctx().msg.data.data(),
         get_validate_ctx().msg.data.size());
   return buffer.size();
}

void requireScope_(uint64_t account) {
   get_validate_ctx().require_scope(Name(account));
}

void requireNotice_(uint64_t account) {
   get_validate_ctx().require_recipient(Name(account));
}

#define RETURN_UPDATE_RECORD(NAME,VALUE_OBJECT) \
   return ctx.NAME##_record<VALUE_OBJECT>(Name(scope), Name(ctx.code.value),Name(table), (VALUE_OBJECT::key_type*)keys, value, valuelen)

#define RETURN_READ_RECORD(NAME) \
      FC_ASSERT(key_value_index::value_type::number_of_keys > scope_index,"scope index out off bound"); \
      return ctx.NAME##_record<key_value_index,by_scope_primary>(Name(scope), Name(code),Name(table), (key_value_index::value_type::key_type*)keys, value, valuelen);

#define RETURN_READ_RECORD_KEY128x128(NAME) \
      FC_ASSERT(key128x128_value_index::value_type::number_of_keys > scope_index,"scope index out off bound"); \
      if(scope_index == 0){ \
         return ctx.NAME##_record<key128x128_value_index,by_scope_primary>(Name(scope), Name(code),Name(table), (key128x128_value_index::value_type::key_type*)keys, value, valuelen); \
      }else if(scope_index == 1){ \
         return ctx.NAME##_record<key128x128_value_index,by_scope_secondary>(Name(scope), Name(code),Name(table), (key128x128_value_index::value_type::key_type*)keys, value, valuelen); \
      }

#define RETURN_READ_RECORD_KEY64x64x64(NAME) \
      FC_ASSERT(key64x64x64_value_index::value_type::number_of_keys > scope_index,"scope index out off bound"); \
      if(scope_index == 0){ \
         return ctx.NAME##_record<key64x64x64_value_index,by_scope_primary>(Name(scope), Name(code),Name(table), (key64x64x64_value_index::value_type::key_type*)keys, value, valuelen); \
      }else if(scope_index == 1){ \
         return ctx.NAME##_record<key64x64x64_value_index,by_scope_secondary>(Name(scope), Name(code),Name(table), (key64x64x64_value_index::value_type::key_type*)keys, value, valuelen); \
      }else if (scope_index == 2){ \
         return ctx.NAME##_record<key64x64x64_value_index,by_scope_tertiary>(Name(scope), Name(code),Name(table), (key64x64x64_value_index::value_type::key_type*)keys, value, valuelen); \
      }

//      FC_ASSERT(VALUE_INDEX::value_type::number_of_keys<scope_index,"scope index out off bound");

int32_t store_(Name scope, Name code, Name table, void *keys, int key_type,
      char *value, uint32_t valuelen) {
//   key128x128_value_index
   apply_context& ctx = get_apply_ctx();
//   return ctx.store_record<key_value_object>(Name(scope), Name(ctx.code.value),Name(table), (key_value_object::key_type*)keys, data, valuelen);

   if (key_type == 0) {
      RETURN_UPDATE_RECORD(store, key_value_object);
   } else if (key_type == 1) {
      RETURN_UPDATE_RECORD(store, key128x128_value_object);
   } else if (key_type == 2) {
      RETURN_UPDATE_RECORD(store, key64x64x64_value_object);
   }
   return 0;
   /*
    key_value_object
    key128x128_value_index
    key64x64x64_value_index
    */
}

int32_t update_(Name scope, Name code, Name table, void *keys, int key_type,
      char *value, uint32_t valuelen) {
   apply_context& ctx = get_apply_ctx();
   if (key_type == 0) {
      RETURN_UPDATE_RECORD(update, key_value_object);
   } else if (key_type == 1) {
      RETURN_UPDATE_RECORD(update, key128x128_value_object);
   } else if (key_type == 2) {
      RETURN_UPDATE_RECORD(update, key64x64x64_value_object);
   }

   return 0;
}

int32_t remove_(Name scope, Name code, Name table, void *keys, int key_type,
      char *value, uint32_t valuelen) {
   apply_context& ctx = get_apply_ctx();
   if (key_type == 0) {
      RETURN_UPDATE_RECORD(remove, key_value_object);
   } else if (key_type == 1) {
      RETURN_UPDATE_RECORD(remove, key128x128_value_object);
   } else if (key_type == 2) {
      RETURN_UPDATE_RECORD(remove, key64x64x64_value_object);
   }
   return 0;
}

int32_t load_(Name scope, Name code, Name table, void *keys, int key_type,
      int scope_index, char *value, uint32_t valuelen) {

   FC_ASSERT(scope_index >= 0, "scope index must be >= 0");

   apply_context& ctx = get_apply_ctx();

   if (key_type == 0) {
      RETURN_READ_RECORD(load);
   } else if (key_type == 1) {
      RETURN_READ_RECORD_KEY128x128(load);
   } else if (key_type == 2) {
      RETURN_READ_RECORD_KEY64x64x64(load);
   }
   return 0;
}

int32_t front_(Name scope, Name code, Name table, void *keys, int key_type,
      int scope_index, char *value, uint32_t valuelen) {

   FC_ASSERT(scope_index >= 0, "scope index must be >= 0");

   apply_context& ctx = get_apply_ctx();

   if (key_type == 0) {
      RETURN_READ_RECORD(front);
   } else if (key_type == 1) {
      RETURN_READ_RECORD_KEY128x128(front);
   } else if (key_type == 2) {
      RETURN_READ_RECORD_KEY64x64x64(front);
   }
   return 0;
}

int32_t back_(Name scope, Name code, Name table, void *keys, int key_type,
      int scope_index, char *value, uint32_t valuelen) {

   FC_ASSERT(scope_index >= 0, "scope index must be >= 0");

   apply_context& ctx = get_apply_ctx();

   if (key_type == 0) {
      RETURN_READ_RECORD(back);
   } else if (key_type == 1) {
      RETURN_READ_RECORD_KEY128x128(back);
   } else if (key_type == 2) {
      RETURN_READ_RECORD_KEY64x64x64(back);
   }
   return 0;
}

int32_t next_(Name scope, Name code, Name table, void *keys, int key_type,
      int scope_index, char *value, uint32_t valuelen) {

   FC_ASSERT(scope_index >= 0, "scope index must be >= 0");

   apply_context& ctx = get_apply_ctx();

   if (key_type == 0) {
      RETURN_READ_RECORD(next);
   } else if (key_type == 1) {
      RETURN_READ_RECORD_KEY128x128(next);
   } else if (key_type == 2) {
      RETURN_READ_RECORD_KEY64x64x64(next);
   }
   return 0;
}

int32_t previous_(Name scope, Name code, Name table, void *keys, int key_type,
      int scope_index, char *value, uint32_t valuelen) {

   FC_ASSERT(scope_index >= 0, "scope index must be >= 0");
   apply_context& ctx = get_apply_ctx();
   if (key_type == 0) {
      RETURN_READ_RECORD(previous);
   } else if (key_type == 1) {
      RETURN_READ_RECORD_KEY128x128(previous);
   } else if (key_type == 2) {
      RETURN_READ_RECORD_KEY64x64x64(previous);
   }
   return 0;
}

int32_t lower_bound_(Name scope, Name code, Name table, void *keys,
      int key_type, int scope_index, char *value, uint32_t valuelen) {
   FC_ASSERT(scope_index >= 0, "scope index must be >= 0");
   apply_context& ctx = get_apply_ctx();
   if (key_type == 0) {
      RETURN_READ_RECORD(lower_bound);
   } else if (key_type == 1) {
      RETURN_READ_RECORD_KEY128x128(lower_bound);
   } else if (key_type == 2) {
      RETURN_READ_RECORD_KEY64x64x64(lower_bound);
   }
   return 0;
}

int32_t upper_bound_(Name scope, Name code, Name table, void *keys,
      int key_type, int scope_index, char *value, uint32_t valuelen) {
   apply_context& ctx = get_apply_ctx();

   FC_ASSERT(scope_index >= 0, "scope index must be >= 0");

   if (key_type == 0) {
      RETURN_READ_RECORD(upper_bound);
   } else if (key_type == 1) {
      RETURN_READ_RECORD_KEY128x128(upper_bound);
   } else if (key_type == 2) {
      RETURN_READ_RECORD_KEY64x64x64(upper_bound);
   }
   return 0;
}

