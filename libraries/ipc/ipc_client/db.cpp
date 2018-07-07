//#include "db_api.hpp"

#warning TODO: Implementing get_code
uint64_t get_code() {
   return 0;
}

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

int32_t db_store_i64(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id,  const void* data, uint32_t len) {
   return ipc_client::get().db_store_i64(scope, table, payer, id,  data, len);
}

int32_t db_store_i64_ex(uint64_t code, uint64_t scope, uint64_t table, uint64_t payer, uint64_t id,  const void* data, uint32_t len) {
   FC_ASSERT(false, "not implemented!");
   return -1;//ctx().db_store_i64(code, scope, table, payer, id,  (const char*)data, len);
}

void db_update_i64(int32_t iterator, uint64_t payer, const void* data, uint32_t len) {
   ipc_client::get().db_update_i64(iterator, payer, (const char*)data, len);
}

void db_update_i64_ex( uint64_t scope, uint64_t payer, uint64_t table, uint64_t id, const char* buffer, size_t buffer_size ) {
   FC_ASSERT(false, "not implemented!");
}

void db_remove_i64(int32_t iterator) {
   return ipc_client::get().db_remove_i64(iterator);
}

void db_remove_i64_ex( uint64_t scope, uint64_t payer, uint64_t table, uint64_t id ) {
   FC_ASSERT(false, "not implemented!");
}

int32_t db_get_i64(int32_t iterator, void* data, uint32_t len) {
   return ipc_client::get().db_get_i64(iterator, (char*)data, len);
}

int32_t db_get_i64_ex( int itr, uint64_t* primary, char* buffer, size_t buffer_size ) {
   return ipc_client::get().db_get_i64_ex( itr, primary, buffer, buffer_size );
}

const char* db_get_i64_exex( int itr, size_t* buffer_size ) {
   return ipc_client::get().db_get_i64_exex( itr,  buffer_size);
}

int32_t db_next_i64(int32_t iterator, uint64_t* primary) {
   return ipc_client::get().db_next_i64(iterator, primary);
}

int32_t db_previous_i64(int32_t iterator, uint64_t* primary) {
   return ipc_client::get().db_previous_i64(iterator, primary);
}

int32_t db_find_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id) {
   return ipc_client::get().db_find_i64(code, scope, table, id);
}

int32_t db_lowerbound_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id) {
   return ipc_client::get().db_lowerbound_i64(code, scope, table, id);
}

int32_t db_upperbound_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id) {
   return ipc_client::get().db_upperbound_i64(code, scope, table, id);
}

int32_t db_end_i64(uint64_t code, uint64_t scope, uint64_t table) {
   return ipc_client::get().db_end_i64(code, scope, table);
}


}


