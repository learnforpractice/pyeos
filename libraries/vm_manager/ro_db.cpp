#include <eosio/chain/db_api.hpp>
#include <fc/exception/exception.hpp>

using namespace eosio::chain;

bool is_nan( const float32_t f ) {
   return ((f.v & 0x7FFFFFFF) > 0x7F800000);
}

bool is_nan( const float64_t f ) {
   return ((f.v & 0x7FFFFFFFFFFFFFFF) > 0x7FF0000000000000);
}

bool is_nan( const float128_t& f ) {
   return (((~(f.v[1]) & uint64_t( 0x7FFF000000000000 )) == 0) && (f.v[0] || ((f.v[1]) & uint64_t( 0x0000FFFFFFFFFFFF ))));
}

extern "C" {

void  eosio_assert( uint32_t test, const char* msg ) {
   if( BOOST_UNLIKELY( !test ) ) {
      std::string message( msg );
      edump((message));
      EOS_THROW( eosio_assert_message_exception, "assertion failure with message: ${s}", ("s",message) );
   }
}

int rodb_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   FC_ASSERT(false, "read only database!");
   return -1;
}

void rodb_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
   FC_ASSERT(false, "read only database!");
}

void rodb_remove_i64 ( int itr ) {
   FC_ASSERT(false, "read only database!");
}

int rodb_get_i64( int itr, char* buffer, size_t buffer_size ) {
   return db_api::get().db_get_i64( itr, buffer, buffer_size );
}

int rodb_next_i64( int itr, uint64_t* primary ) {
   return db_api::get().db_next_i64(itr, *primary);
}

int rodb_previous_i64( int itr, uint64_t* primary ) {
   return db_api::get().db_previous_i64(itr, *primary);
}

int rodb_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api::get().db_find_i64( code, scope, table, id );
}

int rodb_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api::get().db_lowerbound_i64( code, scope, table, id );
}

int rodb_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api::get().db_upperbound_i64( code, scope, table, id );
}

int rodb_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
   return db_api::get().db_end_i64( code, scope, table );
}



#define DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY_(IDX, TYPE)\
      int rodb_##IDX##_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const TYPE* secondary ) {\
         return db_api::get().IDX.store( scope, table, payer, id, *secondary );\
      }\
      void rodb_##IDX##_update( int iterator, uint64_t payer, const TYPE* secondary ) {\
         return db_api::get().IDX.update( iterator, payer, *secondary );\
      }\
      void rodb_##IDX##_remove( int iterator ) {\
         return db_api::get().IDX.remove( iterator );\
      }\
      int rodb_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const TYPE* secondary, uint64_t* primary ) {\
         return db_api::get().IDX.find_secondary(code, scope, table, *secondary, *primary);\
      }\
      int rodb_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, TYPE* secondary, uint64_t primary ) {\
         return db_api::get().IDX.find_primary(code, scope, table, *secondary, primary);\
      }\
      int rodb_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE* secondary, uint64_t* primary ) {\
         return db_api::get().IDX.lowerbound_secondary(code, scope, table, *secondary, *primary);\
      }\
      int rodb_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE* secondary, uint64_t* primary ) {\
         return db_api::get().IDX.upperbound_secondary(code, scope, table, *secondary, *primary);\
      }\
      int rodb_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return db_api::get().IDX.end_secondary(code, scope, table);\
      }\
      int rodb_##IDX##_next( int iterator, uint64_t* primary  ) {\
         return db_api::get().IDX.next_secondary(iterator, *primary);\
      }\
      int rodb_##IDX##_previous( int iterator, uint64_t* primary ) {\
         return db_api::get().IDX.previous_secondary(iterator, *primary);\
      }

#define DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY_(IDX, ARR_SIZE, ARR_ELEMENT_TYPE)\
      int rodb_##IDX##_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const ARR_ELEMENT_TYPE* data, size_t data_len) {\
         FC_ASSERT(false, "read only database!"); \
         return -1; \
      }\
      void rodb_##IDX##_update( int iterator, uint64_t payer, const void* data, size_t data_len ) {\
         FC_ASSERT(false, "read only database!"); \
      }\
      void rodb_##IDX##_remove( int iterator ) {\
         FC_ASSERT(false, "read only database!"); \
      }\
      int rodb_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const ARR_ELEMENT_TYPE* data, size_t data_len, uint64_t* primary ) {\
         EOS_ASSERT( data_len == ARR_SIZE,\
              db_api_exception,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return db_api::get().IDX.find_secondary(code, scope, table, (const ARR_ELEMENT_TYPE*)data, *primary);\
      }\
      int rodb_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, ARR_ELEMENT_TYPE* data, size_t data_len, uint64_t primary ) {\
         EOS_ASSERT( data_len == ARR_SIZE,\
              db_api_exception,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return db_api::get().IDX.find_primary(code, scope, table, (ARR_ELEMENT_TYPE*)data, primary);\
      }\
      int rodb_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table, ARR_ELEMENT_TYPE* data, size_t data_len, uint64_t* primary ) {\
         EOS_ASSERT( data_len == ARR_SIZE,\
              db_api_exception,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return db_api::get().IDX.lowerbound_secondary(code, scope, table, (ARR_ELEMENT_TYPE*)data, *primary);\
      }\
      int rodb_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table, ARR_ELEMENT_TYPE* data, size_t data_len, uint64_t* primary ) {\
         EOS_ASSERT( data_len == ARR_SIZE,\
              db_api_exception,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return db_api::get().IDX.upperbound_secondary(code, scope, table, (ARR_ELEMENT_TYPE*)data, *primary);\
      }\
      int rodb_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return db_api::get().IDX.end_secondary(code, scope, table);\
      }\
      int rodb_##IDX##_next( int iterator, uint64_t* primary  ) {\
         return db_api::get().IDX.next_secondary(iterator, *primary);\
      }\
      int rodb_##IDX##_previous( int iterator, uint64_t* primary ) {\
         return db_api::get().IDX.previous_secondary(iterator, *primary);\
      }

#define DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY_(IDX, TYPE)\
      int rodb_##IDX##_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const TYPE* secondary ) {\
         FC_ASSERT(false, "read only database!"); \
         return -1; \
      }\
      void rodb_##IDX##_update( int iterator, uint64_t payer, const TYPE* secondary ) {\
         FC_ASSERT(false, "read only database!"); \
      }\
      void rodb_##IDX##_remove( int iterator ) {\
         FC_ASSERT(false, "read only database!"); \
      }\
      int rodb_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const TYPE* secondary, uint64_t* primary ) {\
         EOS_ASSERT( !is_nan( *secondary ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return db_api::get().IDX.find_secondary(code, scope, table, *secondary, *primary);\
      }\
      int rodb_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, TYPE* secondary, uint64_t primary ) {\
         return db_api::get().IDX.find_primary(code, scope, table, *secondary, primary);\
      }\
      int rodb_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE* secondary, uint64_t* primary ) {\
         EOS_ASSERT( !is_nan( *secondary ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return db_api::get().IDX.lowerbound_secondary(code, scope, table, *secondary, *primary);\
      }\
      int rodb_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE* secondary, uint64_t* primary ) {\
         EOS_ASSERT( !is_nan( *secondary ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return db_api::get().IDX.upperbound_secondary(code, scope, table, *secondary, *primary);\
      }\
      int rodb_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return db_api::get().IDX.end_secondary(code, scope, table);\
      }\
      int rodb_##IDX##_next( int iterator, uint64_t* primary  ) {\
         return db_api::get().IDX.next_secondary(iterator, *primary);\
      }\
      int rodb_##IDX##_previous( int iterator, uint64_t* primary ) {\
         return db_api::get().IDX.previous_secondary(iterator, *primary);\
      }

DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY_(idx64,  uint64_t)
DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY_(idx128, uint128_t)
DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY_(idx256, 2, uint128_t)
DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY_(idx_double, float64_t)
DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY_(idx_long_double, float128_t)

}
