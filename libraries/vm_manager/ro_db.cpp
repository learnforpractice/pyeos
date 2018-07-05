#include <eosio/chain/db_api.hpp>
using namespace eosio::chain;

extern "C" {

void  eosio_assert( uint32_t test, const char* msg );

int rodb_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   eosio_assert(false, "read only database!");
   return -1;
}

void rodb_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
   eosio_assert(false, "read only database!");
}

void rodb_remove_i64 ( int itr ) {
   eosio_assert(false, "read only database!");
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

}
