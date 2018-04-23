#include <algorithm>
#include <eosio/chain/chain_controller.hpp>
#include <eosio/chain/wasm_interface.hpp>
#include <eosio/chain/micropython_interface.hpp>

#include <eosio/chain/generated_transaction_object.hpp>
#include <eosio/chain/scope_sequence_object.hpp>
#include <boost/container/flat_set.hpp>


#include <eosio/chain/account_object.hpp>
#include <eosio/chain/permission_object.hpp>
#include <eosio/chain/action_objects.hpp>
#include <eosio/chain/block_summary_object.hpp>
#include <eosio/chain/generated_transaction_object.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/permission_link_object.hpp>
#include <eosio/chain/producer_object.hpp>
#include <eosio/chain/transaction_object.hpp>
#include <eosio/chain/scope_sequence_object.hpp>
#include <eosio/chain/permission_object.hpp>

#include "database_api.hpp"



extern "C" {
#include <stdio.h>
#include <string.h>

#include "py/objlist.h"
#include "py/objstringio.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/obj.h"
#include "py/compile.h"
#include "py/gc.h"
}

using boost::container::flat_set;

extern "C" void execution_start();
extern "C" void execution_end();

namespace eosio { namespace chain {

database_api *database_api::_instance = 0;

database_api::database_api(const action& a)
: db(fc::path("data-dir/shared_mem"), chainbase::database::read_only, config::default_shared_memory_size),
 act(a),
 used_authorizations(act.authorization.size(), false),
 idx64(*this),
 idx128(*this),
 idx256(*this),
 idx_double(*this),
 _cpu_usage(0)
{
   db.add_index<account_index>();
   db.add_index<permission_index>();
   db.add_index<permission_usage_index>();
   db.add_index<permission_link_index>();
   db.add_index<action_permission_index>();

   db.add_index<contracts::table_id_multi_index>();
   db.add_index<contracts::key_value_index>();
   db.add_index<contracts::index64_index>();
   db.add_index<contracts::index128_index>();
   db.add_index<contracts::index256_index>();
   db.add_index<contracts::index_double_index>();

   db.add_index<global_property_multi_index>();
   db.add_index<dynamic_global_property_multi_index>();
   db.add_index<block_summary_multi_index>();
   db.add_index<transaction_multi_index>();
   db.add_index<generated_transaction_multi_index>();
   db.add_index<producer_multi_index>();
   db.add_index<scope_sequence_multi_index>();
}

void database_api::get_code(uint64_t account, string& code) {
   const auto &a = db.get<account_object, by_name>(account);
   code = string(a.code.data(), a.code.size());
}

bool database_api::is_account(const account_name& account)const {
   return nullptr != db.find<account_object,by_name>( account );
}

bool database_api::all_authorizations_used()const {
   for ( bool has_auth : used_authorizations ) {
      if ( !has_auth )
         return false;
   }
   return true;
}

vector<permission_level> database_api::unused_authorizations()const {
   vector<permission_level> ret_auths;
   for ( uint32_t i=0; i < act.authorization.size(); i++ )
      if ( !used_authorizations[i] )
         ret_auths.push_back( act.authorization[i] );
   return ret_auths;
}

void database_api::require_authorization( const account_name& account ) {
   for( uint32_t i=0; i < act.authorization.size(); i++ ) {
     if( act.authorization[i].actor == account ) {
        used_authorizations[i] = true;
        return;
     }
   }
   EOS_ASSERT( false, tx_missing_auth, "missing authority of ${account}", ("account",account));
}

bool database_api::has_authorization( const account_name& account )const {
   for( const auto& auth : act.authorization )
     if( auth.actor == account )
        return true;
  return false;
}

void database_api::require_authorization(const account_name& account,
                                          const permission_name& permission) {
  for( uint32_t i=0; i < act.authorization.size(); i++ )
     if( act.authorization[i].actor == account ) {
        if( act.authorization[i].permission == permission ) {
           used_authorizations[i] = true;
           return;
        }
     }
  EOS_ASSERT( false, tx_missing_auth, "missing authority of ${account}/${permission}",
              ("account",account)("permission",permission) );
}

void database_api::require_read_lock(const account_name& account, const scope_name& scope) {
   /*
   if (trx_meta.allowed_read_locks || trx_meta.allowed_write_locks ) {
      bool locked_for_read = trx_meta.allowed_read_locks && locks_contain(**trx_meta.allowed_read_locks, account, scope);
      if (!locked_for_read && trx_meta.allowed_write_locks) {
         locked_for_read = locks_contain(**trx_meta.allowed_write_locks, account, scope);
      }
      EOS_ASSERT( locked_for_read , block_lock_exception, "read lock \"${a}::${s}\" required but not provided", ("a", account)("s",scope) );
   }

   if (!locks_contain(_read_locks, account, scope)) {
      _read_locks.emplace_back(shard_lock{account, scope});
   }
   */
}

const contracts::table_id_object* database_api::find_table( name code, name scope, name table ) {
   require_read_lock(code, scope);
   return db.find<table_id_object, contracts::by_code_scope_table>(boost::make_tuple(code, scope, table));
}

int database_api::db_get_i64( int iterator, char* buffer, size_t buffer_size ) {
   const key_value_object& obj = keyval_cache.get( iterator );
   memcpy( buffer, obj.value.data(), std::min(obj.value.size(), buffer_size) );

   return obj.value.size();
}

int database_api::db_next_i64( int iterator, uint64_t& primary ) {
   if( iterator < -1 ) return -1; // cannot increment past end iterator of table

   const auto& obj = keyval_cache.get( iterator ); // Check for iterator != -1 happens in this call
   const auto& idx = db.get_index<contracts::key_value_index, contracts::by_scope_primary>();

   auto itr = idx.iterator_to( obj );
   ++itr;

   if( itr == idx.end() || itr->t_id != obj.t_id ) return keyval_cache.get_end_iterator_by_table_id(obj.t_id);

   primary = itr->primary_key;
   return keyval_cache.add( *itr );
}

int database_api::db_previous_i64( int iterator, uint64_t& primary ) {
   const auto& idx = db.get_index<contracts::key_value_index, contracts::by_scope_primary>();

   if( iterator < -1 ) // is end iterator
   {
      auto tab = keyval_cache.find_table_by_end_iterator(iterator);
      FC_ASSERT( tab, "not a valid end iterator" );

      auto itr = idx.upper_bound(tab->id);
      if( idx.begin() == idx.end() || itr == idx.begin() ) return -1; // Empty table

      --itr;

      if( itr->t_id != tab->id ) return -1; // Empty table

      primary = itr->primary_key;
      return keyval_cache.add(*itr);
   }

   const auto& obj = keyval_cache.get(iterator); // Check for iterator != -1 happens in this call

   auto itr = idx.iterator_to(obj);
   if( itr == idx.begin() ) return -1; // cannot decrement past beginning iterator of table

   --itr;

   if( itr->t_id != obj.t_id ) return -1; // cannot decrement past beginning iterator of table

   primary = itr->primary_key;
   return keyval_cache.add(*itr);
}

int database_api::db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   require_read_lock( code, scope ); // redundant?

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const key_value_object* obj = db.find<key_value_object, contracts::by_scope_primary>( boost::make_tuple( tab->id, id ) );
   if( !obj ) return table_end_itr;

   return keyval_cache.add( *obj );
}

int database_api::db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   require_read_lock( code, scope ); // redundant?

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const auto& idx = db.get_index<contracts::key_value_index, contracts::by_scope_primary>();
   auto itr = idx.lower_bound( boost::make_tuple( tab->id, id ) );
   if( itr == idx.end() ) return table_end_itr;
   if( itr->t_id != tab->id ) return table_end_itr;

   return keyval_cache.add( *itr );
}

int database_api::db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   require_read_lock( code, scope ); // redundant?

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const auto& idx = db.get_index<contracts::key_value_index, contracts::by_scope_primary>();
   auto itr = idx.upper_bound( boost::make_tuple( tab->id, id ) );
   if( itr == idx.end() ) return table_end_itr;
   if( itr->t_id != tab->id ) return table_end_itr;

   return keyval_cache.add( *itr );
}

int database_api::db_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
   require_read_lock( code, scope ); // redundant?

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   return keyval_cache.cache_table( *tab );
}


#define DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(IDX, TYPE)\
      int database_api::db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const TYPE& secondary, uint64_t& primary ) {\
         return IDX.find_secondary(code, scope, table, secondary, primary);\
      }\
      int database_api::db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, TYPE& secondary, uint64_t primary ) {\
         return IDX.find_primary(code, scope, table, secondary, primary);\
      }\
      int database_api::db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE& secondary, uint64_t& primary ) {\
         return IDX.lowerbound_secondary(code, scope, table, secondary, primary);\
      }\
      int database_api::db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE& secondary, uint64_t& primary ) {\
         return IDX.upperbound_secondary(code, scope, table, secondary, primary);\
      }\
      int database_api::db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return IDX.end_secondary(code, scope, table);\
      }\
      int database_api::db_##IDX##_next( int iterator, uint64_t& primary  ) {\
         return IDX.next_secondary(iterator, primary);\
      }\
      int database_api::db_##IDX##_previous( int iterator, uint64_t& primary ) {\
         return IDX.previous_secondary(iterator, primary);\
      }

#define DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY(IDX, ARR_SIZE, ARR_ELEMENT_TYPE)\
      int database_api::db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, array_ptr<const ARR_ELEMENT_TYPE> data, size_t data_len, uint64_t& primary ) {\
         FC_ASSERT( data_len == ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return IDX.find_secondary(code, scope, table, data, primary);\
      }\
      int database_api::db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, array_ptr<ARR_ELEMENT_TYPE> data, size_t data_len, uint64_t primary ) {\
         FC_ASSERT( data_len == ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return IDX.find_primary(code, scope, table, data.value, primary);\
      }\
      int database_api::db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table, array_ptr<ARR_ELEMENT_TYPE> data, size_t data_len, uint64_t& primary ) {\
         FC_ASSERT( data_len == ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return IDX.lowerbound_secondary(code, scope, table, data.value, primary);\
      }\
      int database_api::db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table, array_ptr<ARR_ELEMENT_TYPE> data, size_t data_len, uint64_t& primary ) {\
         FC_ASSERT( data_len == ARR_SIZE,\
                    "invalid size of secondary key array for " #IDX ": given ${given} bytes but expected ${expected} bytes",\
                    ("given",data_len)("expected",ARR_SIZE) );\
         return IDX.upperbound_secondary(code, scope, table, data.value, primary);\
      }\
      int database_api::db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return IDX.end_secondary(code, scope, table);\
      }\
      int database_api::db_##IDX##_next( int iterator, uint64_t& primary  ) {\
         return IDX.next_secondary(iterator, primary);\
      }\
      int database_api::db_##IDX##_previous( int iterator, uint64_t& primary ) {\
         return IDX.previous_secondary(iterator, primary);\
      }

#define DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY(IDX, TYPE)\
      int database_api::db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const char* secondary, size_t data_len, uint64_t* primary ) {\
         EOS_ASSERT( !softfloat_api::is_nan( *((float64_t*)secondary) ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return IDX.find_secondary(code, scope, table, *((float64_t*)secondary), *primary);\
      }\
      int database_api::db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, char* secondary, size_t data_len, uint64_t primary ) {\
         return IDX.find_primary(code, scope, table, *((float64_t*)secondary), primary);\
      }\
      int database_api::db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  char* secondary, size_t data_len, uint64_t* primary ) {\
         EOS_ASSERT( !softfloat_api::is_nan( *((float64_t*)secondary) ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return IDX.lowerbound_secondary(code, scope, table, *((float64_t*)secondary), *primary);\
      }\
      int database_api::db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table,  char* secondary, size_t data_len, uint64_t* primary ) {\
         EOS_ASSERT( !softfloat_api::is_nan( *((float64_t*)secondary) ), transaction_exception, "NaN is not an allowed value for a secondary key" );\
         return IDX.upperbound_secondary(code, scope, table, *((float64_t*)secondary), *primary);\
      }\
      int database_api::db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ) {\
         return IDX.end_secondary(code, scope, table);\
      }\
      int database_api::db_##IDX##_next( int iterator, uint64_t* primary  ) {\
         return IDX.next_secondary(iterator, *primary);\
      }\
      int database_api::db_##IDX##_previous( int iterator, uint64_t* primary ) {\
         return IDX.previous_secondary(iterator, *primary);\
      }

DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(idx64,  uint64_t)
DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY(idx128, uint128_t)
DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY(idx256, 2, uint128_t)
DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY(idx_double, uint64_t)

} } /// eosio::chain
