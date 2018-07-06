#include "ipc_manager.hpp"

#include <eosio/chain/db_api.hpp>
using namespace eosio::chain;

void ipc_manager::require_recipient( uint64_t name ) {

}

void ipc_manager::require_auth( uint64_t name ) {

}

void ipc_manager::require_auth2( uint64_t name, uint64_t permission ) {

}

void ipc_manager::send_inline(char *data, size_t data_len) {

}

void ipc_manager::send_context_free_inline(char *data, size_t data_len) {

}

uint64_t ipc_manager::publication_time() {
   return 0;
}

uint32_t ipc_manager::get_active_producers( uint64_t* producers, uint32_t buffer_size ) {
   return 0;
}

int32_t ipc_manager::db_store_i64(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id,  const void* data, uint32_t len) {
   /*
      std::string _buffer(buffer, buffer_size);
      rpcclient->db_store_i64(scope, table, payer, id, _buffer);
      return db_api::get().db_find_i64(mp_get_receiver(), scope, table, id);
   */
   return -1;
}

void ipc_manager::db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
   uint64_t code;
   uint64_t scope;
   uint64_t _payer;
   uint64_t table;
   uint64_t id;
   db_api::get().db_get_table_i64( itr, code, scope, _payer, table, id );
   std::string _buffer(buffer, buffer_size);
//   rpcclient->db_update_i64_ex( scope, payer, table, id, _buffer);
}


void ipc_manager::db_remove_i64( int itr ) {
   uint64_t code;
   uint64_t scope;
   uint64_t payer;
   uint64_t table;
   uint64_t id;
   db_api::get().db_get_table_i64( itr, code, scope, payer, table, id );
//   rpcclient->db_remove_i64_ex(scope, payer, table, id);
//   db_api::get().db_remove_i64(itr);
}

int32_t ipc_manager::db_get_i64(int32_t iterator, void* data, uint32_t len) {
   return db_api::get().db_get_i64(iterator, (char*)data, len);
}

int32_t ipc_manager::db_get_i64_ex( int itr, uint64_t* primary, char* buffer, size_t buffer_size ) {
   return db_api::get().db_get_i64_ex(itr, *primary, buffer, buffer_size);
}

const char* ipc_manager::db_get_i64_exex( int itr, size_t* buffer_size ) {
   return db_api::get().db_get_i64_exex( itr,  buffer_size);
}

int32_t ipc_manager::db_next_i64(int32_t iterator, uint64_t* primary) {
   return db_api::get().db_next_i64(iterator, *primary);
}

int32_t ipc_manager::db_previous_i64(int32_t iterator, uint64_t* primary) {
   return db_api::get().db_previous_i64(iterator, *primary);
}

int32_t ipc_manager::db_find_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id) {
   return db_api::get().db_find_i64(code, scope, table, id);
}

int32_t ipc_manager::db_lowerbound_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id) {
   return db_api::get().db_lowerbound_i64(code, scope, table, id);
}

int32_t ipc_manager::db_upperbound_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id) {
   return db_api::get().db_upperbound_i64(code, scope, table, id);
}

int32_t ipc_manager::db_end_i64(uint64_t code, uint64_t scope, uint64_t table) {
   return db_api::get().db_end_i64(code, scope, table);
}

int32_t ipc_manager::check_transaction_authorization( const char* trx_data,     uint32_t trx_size,
                                 const char* pubkeys_data, uint32_t pubkeys_size,
                                 const char* perms_data,   uint32_t perms_size
                               ) {
   return 0;
}


