/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#pragma once
#include "micropython/database_api.hpp"


using namespace eosio::chain;

typedef __uint128_t uint128_t_;

void get_code_( uint64_t account, string& code );
bool is_account_( uint64_t account );
uint64_t s2n_(const char* str);
void n2s_(uint64_t n, string& result);

void eosio_assert_(int condition, const char* str);

int db_get_i64_( int iterator, char* buffer, size_t buffer_size );
int db_next_i64_( int iterator, uint64_t& primary );
int db_previous_i64_( int iterator, uint64_t& primary );
int db_find_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_lowerbound_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_upperbound_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_end_i64_( uint64_t code, uint64_t scope, uint64_t table );


#define DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY_WRAP_DEF(IDX, TYPE)\
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const TYPE& secondary, uint64_t& primary );\
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, TYPE& secondary, uint64_t primary );\
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE& secondary, uint64_t& primary );\
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE& secondary, uint64_t& primary );\
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table );\
      int db_##IDX##_next( int iterator, uint64_t& primary  );\
      int db_##IDX##_previous( int iterator, uint64_t& primary );

#define DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY_WRAP_DEF(IDX, ARR_SIZE, ARR_ELEMENT_TYPE)\
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const char* data, size_t data_len, uint64_t& primary );\
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, char*  data, size_t data_len, uint64_t primary );\
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table, char*  data, size_t data_len, uint64_t& primary );\
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table, char*  data, size_t data_len, uint64_t& primary );\
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table );\
      int db_##IDX##_next( int iterator, uint64_t& primary  );\
      int db_##IDX##_previous( int iterator, uint64_t& primary );

#define DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY_DEF(IDX, TYPE)\
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const char* secondary, size_t data_len, uint64_t* primary ); \
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, char* secondary, size_t data_len, uint64_t primary ); \
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  char* secondary, size_t data_len, uint64_t* primary ); \
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table,  char* secondary, size_t data_len, uint64_t* primary ); \
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ); \
      int db_##IDX##_next( int iterator, uint64_t* primary  ); \
      int db_##IDX##_previous( int iterator, uint64_t* primary );

DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY_WRAP_DEF(idx64,  uint64_t)
DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY_WRAP_DEF(idx128, uint128_t_)
DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY_WRAP_DEF(idx256, 2, uint128_t_)
DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY_DEF(idx_double, uint64_t)


//FC_REFLECT(eosio::chain::database_api::apply_results, (applied_actions)(deferred_transaction_requests)(deferred_transactions_count))
