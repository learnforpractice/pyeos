#include "../../goeos/interface/database_.hpp"

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

#include <eosio/chain/fork_database.hpp>
#include <eosio/chain/block_log.hpp>
#include <eosio/chain/authority_checker.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/protocol.hpp>

#include <chainbase/chainbase.hpp>
#include <fc/scoped_exit.hpp>
#include <boost/signals2/signal.hpp>
#include <fc/log/logger.hpp>

#include "../../goeos/interface/pyobject.hpp"

using namespace chainbase;
using namespace fc;
using namespace eosio::chain;

namespace python {
namespace database {

PyObject* database_create(string& path) {
   auto _db = new chainbase::database(fc::path(path), chainbase::database::read_write, 1 * 1024 * 1024);


   return py_new_uint64((uint64_t)_db);
}

PyObject* database_create_account(void* db, string& name) {
   assert(db);
   chainbase::database* _db = (chainbase::database*)db;

   _db->create<account_object>([](account_object& a) { a.name = "billy"; });

   return py_new_none();
}

PyObject* database_get_account(void* db, string& name) {
   assert(db);
   chainbase::database* _db = (chainbase::database*)db;

   try {
      const auto& accnt = _db->get<account_object, by_name>(name);
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }

   return py_new_none();
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
