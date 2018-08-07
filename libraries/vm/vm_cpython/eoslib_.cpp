#include "eoslib_.hpp"

#include <eosio/chain/name.hpp>
#include <fc/io/raw.hpp>

extern "C" {
   void  eosio_assert( uint32_t test, const char* msg );

   uint32_t read_action_data( void* msg, uint32_t buffer_size );
   uint32_t action_data_size();
   bool is_account( uint64_t name );
   void require_auth( uint64_t name );
   void require_recipient( uint64_t name );
   uint64_t string_to_uint64(const char* str);

   int32_t db_store_i64(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id,  const void* data, uint32_t len);
   int32_t db_store_i64_ex(uint64_t code, uint64_t scope, uint64_t table, uint64_t payer, uint64_t id,  const void* data, uint32_t len);

   void db_update_i64(int32_t iterator, uint64_t payer, const void* data, uint32_t len);
   void db_remove_i64(int32_t iterator);

   int32_t db_get_i64(int32_t iterator, void* data, uint32_t len);
   int32_t db_get_i64_ex( int itr, uint64_t* primary, char* buffer, size_t buffer_size );
   const char* db_get_i64_exex( int itr, size_t* buffer_size );

   int32_t db_next_i64(int32_t iterator, uint64_t* primary);
   int32_t db_previous_i64(int32_t iterator, uint64_t* primary);
   int32_t db_find_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id);
   int32_t db_lowerbound_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id);
   int32_t db_upperbound_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id);
   int32_t db_end_i64(uint64_t code, uint64_t scope, uint64_t table);

   int call_set_args(const char* args , int len);
   int call_get_args(char* args , int len);
   uint64_t call(uint64_t account, uint64_t func);

   void send_inline(const char *serialized_action, size_t size);
}

using namespace eosio::chain;

uint64_t s2n_(const char* str) {
   return string_to_uint64(str);
}

void n2s_(uint64_t _name, string& out) {
   out = eosio::chain::name(_name).to_string();
}

void eosio_assert_(int condition, const char* str) {
   eosio_assert(condition, str);
}

void require_auth_(uint64_t account) {
   require_auth(account);
}

void require_recipient_(uint64_t account) {
   require_recipient(account);
}

int is_account_(uint64_t account) {
   return is_account(account);
}

int read_action_(char* memory, size_t size) {
   return read_action_data(memory, size);
}

int action_size_() {
   return action_data_size();
}

int db_store_i64_( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   return db_store_i64( scope, table, payer, id, buffer, buffer_size );
}

void db_update_i64_( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
   db_update_i64( itr, payer, buffer, buffer_size );
}

void db_remove_i64_( int itr ) {
   db_remove_i64(itr);
}

int db_get_i64_( int itr, char* buffer, size_t buffer_size ) {
   return db_get_i64( itr, buffer, buffer_size );
}

int db_next_i64_( int itr, uint64_t* primary ) {
   return db_next_i64( itr, primary );
}

int db_previous_i64_( int itr, uint64_t* primary ) {
   return db_previous_i64( itr, primary );
}

int db_find_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_find_i64( code, scope, table, id );
}

int db_lowerbound_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_lowerbound_i64( code, scope, table, id );
}

int db_upperbound_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_upperbound_i64( code, scope, table, id );
}

int db_end_i64_( uint64_t code, uint64_t scope, uint64_t table ) {
   return db_end_i64( code, scope, table );
}

int call_set_args_(string& args) {
   return call_set_args(args.c_str(), args.size());
}

int call_get_args_(string& args) {
   int args_size = call_get_args(nullptr, 0);
   vector<char> _args(args_size);
   call_get_args(_args.data(), _args.size());
   args = string(_args.data(), _args.size());
   return _args.size();
}

uint64_t call_(uint64_t account, uint64_t func) {
   return call(account, func);
}

int send_inline_(action& act) {
   vector<char> data = fc::raw::pack<action>(act);
   send_inline(data.data(), data.size());
   return 1;
}


void pack_bytes_(string& in, string& out) {
   string raw(in.c_str(),in.length());
   std::vector<char> o = fc::raw::pack<string>(raw);
   fc::raw::pack<std::vector<char>>(o);
   out = string(o.begin(), o.end());
}

void unpack_bytes_(string& in, string& out) {
   string raw(in.c_str(),in.length());
   std::vector<char> v(raw.begin(), raw.end());
   out = fc::raw::unpack<string>(v);
}

