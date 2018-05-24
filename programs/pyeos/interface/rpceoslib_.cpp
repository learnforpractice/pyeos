#include "micropython/database_api.hpp"
#include "eoslib_.hpp"
#include <eosio/chain/exceptions.hpp>

using namespace eosio::chain;

void n2s_(uint64_t n, string& result) {
   try {
      result = name(n).to_string();
   } catch (...) {
   }
}

extern "C" {

void get_code_( uint64_t account, string& code ) {
   database_api::get().get_code( account, code);
}

uint64_t s2n_(const char* str) {
   try {
      return name(str).value;
   } catch (...) {
   }
   return 0;
}

void eosio_assert_(int condition, const char* str) {
   std::string message( str );
   if( !condition ) edump((message));
   FC_ASSERT( condition, "assertion failed: ${s}", ("s",message));
}

int read_action_( char* buffer, size_t size ) {
   const auto& act_obj = database_api::get().get_action_object();
   if (buffer == NULL || size <= 0) {
      return act_obj.data.size();
   }

   if (size > act_obj.data.size()) {
      size = act_obj.data.size();
   }

   memcpy(buffer, act_obj.data.data(), size);
   return size;
}

bool is_account_( uint64_t account ) {
   return database_api::get().is_account( account );
}

void db_remove_i64_(int itr) {
   database_api::get().db_remove_i64(itr);
}

int db_get_i64_( int iterator, char* buffer, size_t buffer_size ) {
   return database_api::get().db_get_i64( iterator, buffer, buffer_size );
}

int db_next_i64_( int iterator, uint64_t* primary ) {
   return database_api::get().db_next_i64( iterator, *primary );
}

int db_previous_i64_( int iterator, uint64_t* primary ) {
   return database_api::get().db_previous_i64( iterator, *primary );
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

}