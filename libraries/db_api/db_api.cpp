#include <eosio/chain/permission_object.hpp>
#include <eosio/chain/block_summary_object.hpp>
#include <eosio/chain/generated_transaction_object.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/permission_link_object.hpp>
#include <eosio/chain/producer_object.hpp>
#include <eosio/chain/transaction_object.hpp>
#include <eosio/chain/permission_object.hpp>

#include <boost/algorithm/string.hpp>

#include <eosio/chain/db_api.hpp>


using boost::container::flat_set;

extern "C" void execution_start();
extern "C" void execution_end();

namespace eosio { namespace chain {

db_api::db_api(const action& a, bool rw)
: db(fc::path("data-dir/state"), rw ? chainbase::database::read_write: chainbase::database::read_only, config::default_state_size, true),
 act(a)
{
   db.add_index<account_index>();
   db.add_index<account_sequence_index>();

   db.add_index<table_id_multi_index>();
   db.add_index<key_value_index>();
   db.add_index<index64_index>();
   db.add_index<index128_index>();
   db.add_index<index256_index>();
   db.add_index<index_double_index>();
   db.add_index<index_long_double_index>();

   db.add_index<global_property_multi_index>();
   db.add_index<dynamic_global_property_multi_index>();
   db.add_index<block_summary_multi_index>();
   db.add_index<transaction_multi_index>();
   db.add_index<generated_transaction_multi_index>();

   db.add_index<action_object_index>();
}

bool db_api::get_action(action& act) {
   const auto &a = db.get<action_object>();

   act.account = a.account;
   act.name = a.name;

   act.authorization.resize(0);
   act.authorization.resize(a.authorization.size());
   memcpy(act.authorization.data(), a.authorization.data(), a.authorization.size());

   act.data.resize(0);
   act.data.resize(a.data.size());
   memcpy(act.data.data(), a.data.data(), a.data.size());

   return true;
}

const name& db_api::get_receiver() {
   const auto &a = db.get<action_object>();
   return a.receiver;
}

const action_object& db_api::get_action_object() const {
   return db.get<action_object>();
}

void db_api::get_code(uint64_t account, string& code) {
   const auto &a = db.get<account_object, by_name>(account);
   code = string(a.code.data(), a.code.size());
}

const shared_string& db_api::get_code(uint64_t account) {
   const auto &a = db.get<account_object, by_name>(account);
   return a.code;
}

digest_type db_api::get_code_id(uint64_t account) {
   const auto &a = db.get<account_object, by_name>(account);
   return a.code_version;
}

bool db_api::is_account(const account_name& account)const {
   return nullptr != db.find<account_object,by_name>( account );
}

void db_api::require_authorization( const account_name& account ) {
   for( uint32_t i=0; i < act.authorization.size(); i++ ) {
     if( act.authorization[i].actor == account ) {
        used_authorizations[i] = true;
        return;
     }
   }
   EOS_ASSERT( false, missing_auth_exception, "missing authority of ${account}", ("account",account));
}

bool db_api::has_authorization( const account_name& account )const {
   for( const auto& auth : act.authorization )
     if( auth.actor == account )
        return true;
  return false;
}

void db_api::require_authorization(const account_name& account,
                                          const permission_name& permission) {
  for( uint32_t i=0; i < act.authorization.size(); i++ )
     if( act.authorization[i].actor == account ) {
        if( act.authorization[i].permission == permission ) {
           used_authorizations[i] = true;
           return;
        }
     }
  EOS_ASSERT( false, missing_auth_exception, "missing authority of ${account}/${permission}",
              ("account",account)("permission",permission) );
}

const table_id_object& db_api::find_or_create_table( name code, name scope, name table, const account_name &payer ) {
//   require_read_lock(code, scope);
   const auto* existing_tid =  db.find<table_id_object, by_code_scope_table>(boost::make_tuple(code, scope, table));
   if (existing_tid != nullptr) {
      return *existing_tid;
   }

//   require_write_lock(scope);

   update_db_usage(payer, config::billable_size_v<table_id_object>);

   return db.create<table_id_object>([&](table_id_object &t_id){
      t_id.code = code;
      t_id.scope = scope;
      t_id.table = table;
      t_id.payer = payer;
   });
}

int db_api::db_store_i64( uint64_t code, uint64_t scope, uint64_t table, const account_name& payer, uint64_t id, const char* buffer, size_t buffer_size ) {
//   require_write_lock( scope );
   const auto& tab = find_or_create_table( code, scope, table, payer );
   auto tableid = tab.id;

   FC_ASSERT( payer != account_name(), "must specify a valid account to pay for new record" );

   const auto& obj = db.create<key_value_object>( [&]( auto& o ) {
      o.t_id        = tableid;
      o.primary_key = id;
      o.value.resize( buffer_size );
      o.payer       = payer;
      memcpy( o.value.data(), buffer, buffer_size );
   });

   db.modify( tab, [&]( auto& t ) {
     ++t.count;
   });

   int64_t billable_size = (int64_t)(buffer_size + config::billable_size_v<key_value_object>);
   update_db_usage( payer, billable_size);

   keyval_cache.cache_table( tab );
   return keyval_cache.add( obj );
}

int db_api::db_store_i64(  uint64_t scope, uint64_t table, const account_name& payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   return db_store_i64(get_receiver(),  scope, table,payer, id, buffer, buffer_size );
}

void db_api::db_update_i64( int iterator, account_name payer, const char* buffer, size_t buffer_size ) {
   const key_value_object& obj = keyval_cache.get( iterator );

   const auto& table_obj = keyval_cache.get_table( obj.t_id );
   FC_ASSERT( table_obj.code == receiver, "db access violation" );

//   require_write_lock( table_obj.scope );

   const int64_t overhead = config::billable_size_v<key_value_object>;
   int64_t old_size = (int64_t)(obj.value.size() + overhead);
   int64_t new_size = (int64_t)(buffer_size + overhead);

   if( payer == account_name() ) payer = obj.payer;

   if( account_name(obj.payer) != payer ) {
      // refund the existing payer
      update_db_usage( obj.payer,  -(old_size) );
      // charge the new payer
      update_db_usage( payer,  (new_size));
   } else if(old_size != new_size) {
      // charge/refund the existing payer the difference
      update_db_usage( obj.payer, new_size - old_size);
   }

   db.modify( obj, [&]( auto& o ) {
     o.value.resize( buffer_size );
     memcpy( o.value.data(), buffer, buffer_size );
     o.payer = payer;
   });
}

void db_api::db_remove_i64( int iterator ) {
   const key_value_object& obj = keyval_cache.get( iterator );

   const auto& table_obj = keyval_cache.get_table( obj.t_id );
   FC_ASSERT( table_obj.code == receiver, "db access violation" );

//   require_write_lock( table_obj.scope );

   update_db_usage( obj.payer,  -(obj.value.size() + config::billable_size_v<key_value_object>) );

   db.modify( table_obj, [&]( auto& t ) {
      --t.count;
   });
   db.remove( obj );

   if (table_obj.count == 0) {
      remove_table(table_obj);
   }

   keyval_cache.remove( iterator );
}

void db_api::remove_table( const table_id_object& tid ) {
   update_db_usage(tid.payer, - config::billable_size_v<table_id_object>);
   db.remove(tid);
}

void db_api::update_db_usage( const account_name& payer, int64_t delta ) {
#if 0
   require_write_lock( payer );
   if( (delta > 0) ) {
      if (!(privileged || payer == account_name(receiver))) {
         require_authorization( payer );
      }

      mutable_controller.get_mutable_resource_limits_manager().add_pending_account_ram_usage(payer, delta);
   }
#endif
}

void db_api::db_remove_i64_ex( int iterator ) {
#if 0
   const key_value_object& obj = keyval_cache.get( iterator );

   update_db_usage( obj.payer,  -(obj.value.size() + config::billable_size_v<key_value_object>) );

   const auto& table_obj = keyval_cache.get_table( obj.t_id );
   FC_ASSERT( table_obj.code == receiver, "db access violation" );

//   require_write_lock( table_obj.scope );

   update_db_usage( obj.payer,  -(obj.value.size() + config::billable_size_v<key_value_object>) );

   db.modify( table_obj, [&]( auto& t ) {
      --t.count;
   });
   db.remove( obj );

   if (table_obj.count == 0) {
      remove_table(table_obj);
   }
#endif
   keyval_cache.remove( iterator );
}

void db_api::db_get_table_i64( int iterator, uint64_t& code, uint64_t& scope, uint64_t& payer, uint64_t& table, uint64_t& id) {
   const key_value_object& obj = keyval_cache.get( iterator );
   const auto& table_obj = keyval_cache.get_table( obj.t_id );

   code = table_obj.code;
   scope = table_obj.scope;
   table = table_obj.table;
   payer = table_obj.payer;
   id = obj.primary_key;
}

const table_id_object* db_api::find_table( name code, name scope, name table ) {
//   require_read_lock(code, scope);
   return db.find<table_id_object, by_code_scope_table>(boost::make_tuple(code, scope, table));
}

int db_api::db_get_i64( int iterator, char* buffer, size_t buffer_size ) {
   const key_value_object& obj = keyval_cache.get( iterator );
   memcpy( buffer, obj.value.data(), std::min(obj.value.size(), buffer_size) );

   return obj.value.size();
}

int db_api::db_get_i64_ex( int iterator, uint64_t& primary, char* buffer, size_t buffer_size ) {
   const key_value_object& obj = keyval_cache.get( iterator );
   memcpy( buffer, obj.value.data(), std::min(obj.value.size(), buffer_size) );

   primary = obj.primary_key;
   return obj.value.size();
}

const char* db_api::db_get_i64_exex( int itr, size_t* buffer_size ) {
   const key_value_object& obj = keyval_cache.get( itr );
   *buffer_size = obj.value.size();
   return obj.value.data();
}

int db_api::db_next_i64( int iterator, uint64_t& primary ) {
   if( iterator < -1 ) return -1; // cannot increment past end iterator of table

   const auto& obj = keyval_cache.get( iterator ); // Check for iterator != -1 happens in this call
   const auto& idx = db.get_index<key_value_index, by_scope_primary>();

   auto itr = idx.iterator_to( obj );
   ++itr;

   if( itr == idx.end() || itr->t_id != obj.t_id ) return keyval_cache.get_end_iterator_by_table_id(obj.t_id);

   primary = itr->primary_key;
   return keyval_cache.add( *itr );
}

int db_api::db_previous_i64( int iterator, uint64_t& primary ) {
   const auto& idx = db.get_index<key_value_index, by_scope_primary>();

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

int db_api::db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
//   require_read_lock( code, scope ); // redundant?

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const key_value_object* obj = db.find<key_value_object, by_scope_primary>( boost::make_tuple( tab->id, id ) );
   if( !obj ) return table_end_itr;

   return keyval_cache.add( *obj );
}

int db_api::db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
//   require_read_lock( code, scope ); // redundant?

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const auto& idx = db.get_index<key_value_index, by_scope_primary>();
   auto itr = idx.lower_bound( boost::make_tuple( tab->id, id ) );
   if( itr == idx.end() ) return table_end_itr;
   if( itr->t_id != tab->id ) return table_end_itr;

   return keyval_cache.add( *itr );
}

int db_api::db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
//   require_read_lock( code, scope ); // redundant?

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const auto& idx = db.get_index<key_value_index, by_scope_primary>();
   auto itr = idx.upper_bound( boost::make_tuple( tab->id, id ) );
   if( itr == idx.end() ) return table_end_itr;
   if( itr->t_id != tab->id ) return table_end_itr;

   return keyval_cache.add( *itr );
}

int db_api::db_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
//   require_read_lock( code, scope ); // redundant?

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   return keyval_cache.cache_table( *tab );
}

bool db_api::is_in_whitelist(uint64_t account) {
   int itr = db_api::get().db_find_i64(N(credit), N(credit), N(credit), account);
   if (itr >= 0) {
      char c = 0;
      int ret = db_api::get().db_get_i64(itr, &c, sizeof(c));
      if (ret == 1) {
         FC_ASSERT(c != '2', "account has been blocked out!");
         if (c == '1') {
            return true;
         }
      }
   }
   return false;
}

} } /// eosio::chain

using namespace eosio::chain;

int mp_is_account2(string& account) {
   account_name _account(account);
   return eosio::chain::db_api::get().is_account(_account);
}

extern "C" {

int mp_action_size() {
   return db_api::get().get_action_object().data.size();
}

int mp_read_action(char* buf, size_t size) {
   const auto& data = db_api::get().get_action_object().data;
   if (size > data.size()) {
      size = data.size();
   }
   memcpy(buf, data.data(), size);
   return size;
}

int mp_is_account(uint64_t account) {
   account_name _account(account);
   return eosio::chain::db_api::get().is_account(_account);
}

uint64_t mp_get_receiver() {
   return db_api::get().get_action_object().receiver;
}

void mp_db_get_table_i64( int itr, uint64_t *code, uint64_t *scope, uint64_t *payer, uint64_t *table, uint64_t *id) {
   db_api::get().db_get_table_i64( itr, *code, *scope, *payer, *table, *id );
}

void db_api_remove_i64(int itr) {
   db_api::get().db_remove_i64( itr );
}

int db_api_get_i64( int itr, char* buffer, size_t buffer_size ) {
   return db_api::get().db_get_i64(itr, buffer, buffer_size);
}

int32_t db_api_get_i64_ex( int iterator, uint64_t* primary, char* buffer, size_t buffer_size ) {
   return db_api::get().db_get_i64_ex(iterator, *primary, buffer, buffer_size);
}

const char* db_api_get_i64_exex( int itr, size_t* buffer_size ) {
   return db_api::get().db_get_i64_exex(itr, buffer_size);
}

int db_api_next_i64( int itr, uint64_t* primary ) {
   return db_api::get().db_next_i64(itr, *primary);
}

int db_api_previous_i64( int itr, uint64_t* primary ) {
   return db_api::get().db_previous_i64(itr, *primary);
}

int db_api_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api::get().db_find_i64(code, scope, table, id);
}

int db_api_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api::get().db_lowerbound_i64(code, scope, table, id);
}

int db_api_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_api::get().db_upperbound_i64(code, scope, table, id);
}

int db_api_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
   return db_api::get().db_end_i64(code, scope, table);
}

}


