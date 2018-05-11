#include "eoslib_.hpp"
#include <eosio/chain/exceptions.hpp>


uint64_t s2n_(const char* str) {
   try {
      return name(str).value;
   } catch (...) {
   }
   return 0;
}

void n2s_(uint64_t n, string& result) {
   try {
      result = name(n).to_string();
   } catch (...) {
   }
}

void eosio_assert_(int condition, const char* str) {
   std::string message( str );
   if( !condition ) edump((message));
   FC_ASSERT( condition, "assertion failed: ${s}", ("s",message));
}

void get_code_( uint64_t account, string& code ) {
   database_api::get().get_code( account, code);
}

bool is_account_( uint64_t account ) {
   return database_api::get().is_account( account );
}


int db_get_i64_( int iterator, char* buffer, size_t buffer_size ) {
   return database_api::get().db_get_i64( iterator, buffer, buffer_size );
}

int db_next_i64_( int iterator, uint64_t& primary ) {
   return database_api::get().db_next_i64( iterator, primary );
}

int db_previous_i64_( int iterator, uint64_t& primary ) {
   return database_api::get().db_previous_i64( iterator, primary );
}

int db_find_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return database_api::get().db_find_i64( code, scope, table, id );
}

int db_lowerbound_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return database_api::get().db_lowerbound_i64( code, scope, table, id );
}

int db_upperbound_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return database_api::get().db_upperbound_i64( code, scope, table, id );
}

int db_end_i64_( uint64_t code, uint64_t scope, uint64_t table ) {
   return database_api::get().db_end_i64( code, scope, table );
}


#define DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY_WRAP(IDX, TYPE)\
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const TYPE& secondary, uint64_t& primary ) {\
         return database_api::get().IDX.find_secondary(code, scope, table, secondary, primary);\
      }\
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, TYPE& secondary, uint64_t primary ) {\
         return database_api::get().IDX.find_primary(code, scope, table, secondary, primary);\
      }\
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE& secondary, uint64_t& primary ) {\
         return database_api::get().IDX.lowerbound_secondary(code, scope, table, secondary, primary);\
      }\
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE& secondary, uint64_t& primary ) {\
         return database_api::get().IDX.upperbound_secondary(code, scope, table, secondary, primary);\
      }\
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return database_api::get().IDX.end_secondary(code, scope, table);\
      }\
      int db_##IDX##_next( int iterator, uint64_t& primary  ) {\
         return database_api::get().IDX.next_secondary(iterator, primary);\
      }\
      int db_##IDX##_previous( int iterator, uint64_t& primary ) {\
         return database_api::get().IDX.previous_secondary(iterator, primary);\
      }

#define DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY_WRAP(IDX, ARR_SIZE, ARR_ELEMENT_TYPE)\
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, array_ptr<const ARR_ELEMENT_TYPE> data, size_t data_len, uint64_t& primary ) {\
         FC_ASSERT( data_len == ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return database_api::get().IDX.find_secondary(code, scope, table, data, primary);\
      }\
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, array_ptr<ARR_ELEMENT_TYPE> data, size_t data_len, uint64_t primary ) {\
         FC_ASSERT( data_len == ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return database_api::get().IDX.find_primary(code, scope, table, data.value, primary);\
      }\
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table, array_ptr<ARR_ELEMENT_TYPE> data, size_t data_len, uint64_t& primary ) {\
         FC_ASSERT( data_len == ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return database_api::get().IDX.lowerbound_secondary(code, scope, table, data.value, primary);\
      }\
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table, array_ptr<ARR_ELEMENT_TYPE> data, size_t data_len, uint64_t& primary ) {\
         FC_ASSERT( data_len == ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return database_api::get().IDX.upperbound_secondary(code, scope, table, data.value, primary);\
      }\
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return database_api::get().IDX.end_secondary(code, scope, table);\
      }\
      int db_##IDX##_next( int iterator, uint64_t& primary  ) {\
         return database_api::get().IDX.next_secondary(iterator, primary);\
      }\
      int db_##IDX##_previous( int iterator, uint64_t& primary ) {\
         return database_api::get().IDX.previous_secondary(iterator, primary);\
      }

#define DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY_WRAP(IDX, TYPE)\
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const char* secondary, size_t data_len, uint64_t* primary ) {\
         EOS_ASSERT( !softfloat_api::is_nan( *((float64_t*)secondary) ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return database_api::get().IDX.find_secondary(code, scope, table, *((float64_t*)secondary), *primary);\
      }\
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, char* secondary, size_t data_len, uint64_t primary ) {\
         return database_api::get().IDX.find_primary(code, scope, table, *((float64_t*)secondary), primary);\
      }\
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  char* secondary, size_t data_len, uint64_t* primary ) {\
         EOS_ASSERT( !softfloat_api::is_nan( *((float64_t*)secondary) ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return database_api::get().IDX.lowerbound_secondary(code, scope, table, *((float64_t*)secondary), *primary);\
      }\
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table,  char* secondary, size_t data_len, uint64_t* primary ) {\
         EOS_ASSERT( !softfloat_api::is_nan( *((float64_t*)secondary) ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return database_api::get().IDX.upperbound_secondary(code, scope, table, *((float64_t*)secondary), *primary);\
      }\
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return database_api::get().IDX.end_secondary(code, scope, table);\
      }\
      int db_##IDX##_next( int iterator, uint64_t* primary  ) {\
         return database_api::get().IDX.next_secondary(iterator, *primary);\
      }\
      int db_##IDX##_previous( int iterator, uint64_t* primary ) {\
         return database_api::get().IDX.previous_secondary(iterator, *primary);\
      }

DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY_WRAP(idx64,  uint64_t)
DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY_WRAP(idx128, uint128_t_)
DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY_WRAP(idx256, 2, uint128_t_)
DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY_WRAP(idx_double, uint64_t)
