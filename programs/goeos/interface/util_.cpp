#include "../../goeos/interface/util_.hpp"

#include "../../goeos/util/db_handling.hpp"

typedef uint64_t TableName;


namespace python {

db_handling& get_db_handling() {
   static db_handling *handling = NULL;
   if (handling == NULL) {
      auto& ctrl = appbase::app().get_plugin<eosio::chain_plugin>().chain();
      handling = new db_handling(*(database*)(&ctrl.get_database()));
   }
   return *handling;
}
#define RETURN_UPDATE_RECORD(NAME, VALUE_OBJECT)       \
   return ctx.NAME##_record<VALUE_OBJECT>(             \
       scope, code, table, \
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

//      FC_ASSERT(VALUE_INDEX::value_type::number_of_keys<scope_index,"scope
//      index out off bound");

int32_t store_(Name scope, Name code, Name table, void* keys, int key_type,
               char* value, uint32_t valuelen) {
   //   key128x128_value_index
   db_handling& ctx = get_db_handling();
   //   return ctx.store_record<key_value_object>(Name(scope),
   //   Name(ctx.code.value),Name(table), (key_value_object::key_type*)keys,
   //   data, valuelen);

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

int32_t update_(Name scope, Name code, Name table, void* keys, int key_type,
                char* value, uint32_t valuelen) {
   db_handling& ctx = get_db_handling();
   if (key_type == 0) {
      RETURN_UPDATE_RECORD(update, key_value_object);
   } else if (key_type == 1) {
      RETURN_UPDATE_RECORD(update, key128x128_value_object);
   } else if (key_type == 2) {
      RETURN_UPDATE_RECORD(update, key64x64x64_value_object);
   }

   return 0;
}

int32_t remove_(Name scope, Name code, Name table, void* keys, int key_type,
                char* value, uint32_t valuelen) {
   db_handling& ctx = get_db_handling();
   if (key_type == 0) {
      RETURN_UPDATE_RECORD(remove, key_value_object);
   } else if (key_type == 1) {
      RETURN_UPDATE_RECORD(remove, key128x128_value_object);
   } else if (key_type == 2) {
      RETURN_UPDATE_RECORD(remove, key64x64x64_value_object);
   }
   return 0;
}

int32_t load_(Name scope, Name code, Name table, void* keys, int key_type,
              int scope_index, char* value, uint32_t valuelen) {
   FC_ASSERT(scope_index >= 0, "scope index must be >= 0");

   db_handling& ctx = get_db_handling();

   if (key_type == 0) {
      RETURN_READ_RECORD(load);
   } else if (key_type == 1) {
      RETURN_READ_RECORD_KEY128x128(load);
   } else if (key_type == 2) {
      RETURN_READ_RECORD_KEY64x64x64(load);
   }
   return 0;
}

int32_t front_(Name scope, Name code, Name table, void* keys, int key_type,
               int scope_index, char* value, uint32_t valuelen) {
   FC_ASSERT(scope_index >= 0, "scope index must be >= 0");

   db_handling& ctx = get_db_handling();

   if (key_type == 0) {
      RETURN_READ_RECORD(front);
   } else if (key_type == 1) {
      RETURN_READ_RECORD_KEY128x128(front);
   } else if (key_type == 2) {
      RETURN_READ_RECORD_KEY64x64x64(front);
   }
   return 0;
}

int32_t back_(Name scope, Name code, Name table, void* keys, int key_type,
              int scope_index, char* value, uint32_t valuelen) {
   FC_ASSERT(scope_index >= 0, "scope index must be >= 0");

   db_handling& ctx = get_db_handling();

   if (key_type == 0) {
      RETURN_READ_RECORD(back);
   } else if (key_type == 1) {
      RETURN_READ_RECORD_KEY128x128(back);
   } else if (key_type == 2) {
      RETURN_READ_RECORD_KEY64x64x64(back);
   }
   return 0;
}

int32_t next_(Name scope, Name code, Name table, void* keys, int key_type,
              int scope_index, char* value, uint32_t valuelen) {
   FC_ASSERT(scope_index >= 0, "scope index must be >= 0");

   db_handling& ctx = get_db_handling();

   if (key_type == 0) {
      RETURN_READ_RECORD(next);
   } else if (key_type == 1) {
      RETURN_READ_RECORD_KEY128x128(next);
   } else if (key_type == 2) {
      RETURN_READ_RECORD_KEY64x64x64(next);
   }
   return 0;
}

int32_t previous_(Name scope, Name code, Name table, void* keys, int key_type,
                  int scope_index, char* value, uint32_t valuelen) {
   FC_ASSERT(scope_index >= 0, "scope index must be >= 0");
   db_handling& ctx = get_db_handling();
   if (key_type == 0) {
      RETURN_READ_RECORD(previous);
   } else if (key_type == 1) {
      RETURN_READ_RECORD_KEY128x128(previous);
   } else if (key_type == 2) {
      RETURN_READ_RECORD_KEY64x64x64(previous);
   }
   return 0;
}

int32_t lower_bound_(Name scope, Name code, Name table, void* keys,
                     int key_type, int scope_index, char* value,
                     uint32_t valuelen) {
   FC_ASSERT(scope_index >= 0, "scope index must be >= 0");
   db_handling& ctx = get_db_handling();
   if (key_type == 0) {
      RETURN_READ_RECORD(lower_bound);
   } else if (key_type == 1) {
      RETURN_READ_RECORD_KEY128x128(lower_bound);
   } else if (key_type == 2) {
      RETURN_READ_RECORD_KEY64x64x64(lower_bound);
   }
   return 0;
}

int32_t upper_bound_(Name scope, Name code, Name table, void* keys,
                     int key_type, int scope_index, char* value,
                     uint32_t valuelen) {
   db_handling& ctx = get_db_handling();

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

}
