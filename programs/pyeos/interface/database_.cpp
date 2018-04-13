#include "database_.hpp"

#include <fc/filesystem.hpp>

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

#include <eosio/chain/fork_database.hpp>
#include <eosio/chain/block_log.hpp>
#include <eosio/chain/authority_checker.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/protocol.hpp>

#include <chainbase/chainbase.hpp>
#include <fc/scoped_exit.hpp>
#include <boost/signals2/signal.hpp>
#include <fc/log/logger.hpp>

#include <eosio/chain/chain_controller.hpp>

#include <eosio/account_history_api_plugin/account_history_api_plugin.hpp>
#include <eosio/chain/config.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/chain/config.hpp>

#include "pyobject.hpp"
#include "json.hpp"

using namespace chainbase;
using namespace fc;
using namespace eosio::chain;

namespace python {
namespace database {

PyObject* database_create(string& path, bool readonly) {
   chainbase::database::open_flags flags;
   if (readonly) {
      flags = chainbase::database::read_only;
   } else {
      flags = chainbase::database::read_write;
   }
   auto _db = new chainbase::database(fc::path(path), flags, config::default_shared_memory_size);

   _db->add_index<account_index>();
   _db->add_index<permission_index>();
   _db->add_index<permission_usage_index>();
   _db->add_index<permission_link_index>();
   _db->add_index<action_permission_index>();



   _db->add_index<contracts::table_id_multi_index>();
   _db->add_index<contracts::key_value_index>();
   _db->add_index<contracts::index64_index>();
   _db->add_index<contracts::index128_index>();
   _db->add_index<contracts::index256_index>();
   _db->add_index<contracts::index_double_index>();

   _db->add_index<global_property_multi_index>();
   _db->add_index<dynamic_global_property_multi_index>();
   _db->add_index<block_summary_multi_index>();
   _db->add_index<transaction_multi_index>();
   _db->add_index<generated_transaction_multi_index>();
   _db->add_index<producer_multi_index>();
   _db->add_index<scope_sequence_multi_index>();

   return py_new_uint64((uint64_t)_db);
}

PyObject* database_create_account(void* db, string& name) {
   assert(db);
   chainbase::database* _db = (chainbase::database*)db;

   _db->create<account_object>([](account_object& a) { a.name = "billy"; });

   return py_new_none();
}

PyObject* database_get_account(void* db, string& _name) {
   assert(db);
   chainbase::database& d = *(chainbase::database*)db;
//   using namespace eosio::contracts;

   eosio::chain::name account_name(_name);

   eosio::chain_apis::read_only::get_account_results result;
   result.account_name = account_name;

   const auto& permissions = d.get_index<permission_index,by_owner>();
   auto perm = permissions.lower_bound( boost::make_tuple( account_name ) );
   while( perm != permissions.end() && perm->owner == account_name ) {
      /// TODO: lookup perm->parent name
      name parent;
      // Don't lookup parent if null
      if( perm->parent._id ) {
         const auto* p = d.find<permission_object,by_id>( perm->parent );
         if( p ) {
            FC_ASSERT(perm->owner == p->owner, "Invalid parent");
            parent = p->name;
         }
      }
      result.permissions.push_back( eosio::chain_apis::permission{ perm->name, parent, perm->auth.to_authority() } );
      ++perm;
   }

   return python::json::to_string(result);
}

PyObject* database_get_recent_transaction(void* db, string& id)
{
   assert(db);
   chainbase::database& _db = *(chainbase::database*)db;

   transaction_id_type trx_id(id);
   auto& index = _db.get_index<transaction_multi_index, by_trx_id>();
   auto itr = index.find(trx_id);
   FC_ASSERT(itr != index.end());
//   return itr->trx;

   return py_new_none();
}

PyObject* database_get_code(void* db, string& code, int& type) {
   assert(db);
   chainbase::database& _db = *(chainbase::database*)db;

   const auto& recipient = _db.get<account_object,by_name>(name(code));
   return py_new_none();
}

PyObject* database_flush(void* db, string& id) {
   assert(db);
   chainbase::database& _db = *(chainbase::database*)db;
   _db.flush();
   return py_new_none();
}

void testtest(void* db) {
   assert(db);
   chainbase::database* _db = (chainbase::database*)db;

   try {
      // Create an account
      _db->create<account_object>([](account_object& a) { a.name = "billy"; });

      // Make sure we can retrieve that account by name
      auto ptr = _db->find<account_object, by_name, std::string>("billy");
      assert(ptr != nullptr);
      _db->flush();

   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
}

}  // namespace database
}  // namespace python
