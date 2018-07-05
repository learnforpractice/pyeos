#include <eosio/chain/db_api.hpp>
using namespace eosio::chain;

extern "C" {

int rwdb_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   return db_api::rwdb().db_store_i64(scope, table, payer, id, buffer, buffer_size);
}

void rwdb_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
   db_api::rwdb().db_update_i64(itr, payer, buffer, buffer_size);
}

void rwdb_remove_i64 ( int itr ) {
   db_api::rwdb().db_remove_i64(itr);
}

int rwdb_get_i64( int itr, char* buffer, size_t buffer_size ) {
   return db_api::rwdb().db_get_i64( itr, buffer, buffer_size );
}

int rwdb_next_i64( int itr, uint64_t* primary ) {
   return db_api::rwdb().db_next_i64(itr, *primary);
}

int rwdb_previous_i64( int itr, uint64_t* primary ) {
   return db_api::rwdb().db_previous_i64(itr, *primary);
}

int rwdb_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api::rwdb().db_find_i64( code, scope, table, id );
}

int rwdb_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api::rwdb().db_lowerbound_i64( code, scope, table, id );
}

int rwdb_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api::rwdb().db_upperbound_i64( code, scope, table, id );
}

int rwdb_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
   return db_api::rwdb().db_end_i64( code, scope, table );
}

}
