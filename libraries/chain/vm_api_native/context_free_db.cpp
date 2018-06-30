#include <eosio/chain/db_api.hpp>
using namespace eosio::chain;
extern "C" {

int db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   return db_api::rwdb().db_store_i64( scope, table, payer, id, buffer, buffer_size );
}

int db_store_i64_ex( uint64_t code, uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   return db_api::rwdb().db_store_i64( code, scope, table, payer, id, buffer, buffer_size );
}

void db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
   db_api::rwdb().db_update_i64( itr, payer, buffer, buffer_size );
}

void db_update_i64_ex( uint64_t scope, uint64_t payer, uint64_t table, uint64_t id, const char* buffer, size_t buffer_size ) {
   int itr = db_api::rwdb().db_find_i64(db_api::rwdb().get_receiver_ex(), scope, table, id);
   if (itr >= 0) {
      db_api::rwdb().db_update_i64( itr, payer, buffer, buffer_size );
   }
}

void db_remove_i64 ( int itr ) {
   db_api::rwdb().db_remove_i64( itr );
}

void db_remove_i64_ex( uint64_t scope, uint64_t payer, uint64_t table, uint64_t id ) {
   int itr = db_api::rwdb().db_find_i64(db_api::rwdb().get_receiver_ex(), scope, table, id);
   if (itr >= 0) {
      db_api::rwdb().db_remove_i64( itr );
   }
}

int db_get_i64( int itr, char* buffer, size_t buffer_size ) {
   return db_api::rwdb().db_get_i64( itr, buffer, buffer_size );
}

int db_get_i64_ex( int itr, uint64_t* primary, char* buffer, size_t buffer_size ) {
   return db_api::rwdb().db_get_i64_ex( itr, *primary, buffer, buffer_size );
}

const char* db_get_i64_exex( int itr, size_t* buffer_size ) {
   return db_api::rwdb().db_get_i64_exex( itr,  buffer_size);
}

int db_next_i64( int itr, uint64_t* primary ) {
   return db_api::rwdb().db_next_i64(itr, *primary);
}

int db_previous_i64( int itr, uint64_t* primary ) {
   return db_api::rwdb().db_previous_i64(itr, *primary);
}

int db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api::rwdb().db_find_i64( code, scope, table, id );
}

int db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api::rwdb().db_lowerbound_i64( code, scope, table, id );
}

int db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api::rwdb().db_upperbound_i64( code, scope, table, id );
}

int db_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
   return db_api::rwdb().db_end_i64( code, scope, table );
}

}
