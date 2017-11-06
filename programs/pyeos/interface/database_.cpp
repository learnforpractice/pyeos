#include "database_.hpp"

#include <fc/filesystem.hpp>

#include <eos/chain/account_object.hpp>
#include <eos/chain/permission_object.hpp>
#include <eos/chain/action_objects.hpp>
#include <eos/chain/block_summary_object.hpp>
#include <eos/chain/generated_transaction_object.hpp>
#include <eos/chain/global_property_object.hpp>
#include <eos/chain/key_value_object.hpp>
#include <eos/chain/permission_link_object.hpp>
#include <eos/chain/producer_object.hpp>
#include <eos/chain/transaction_object.hpp>

#include <eos/chain/fork_database.hpp>
#include <eos/chain/block_log.hpp>
#include <eos/chain/authority_checker.hpp>
#include <eos/chain/block_schedule.hpp>
#include <eos/chain/chain_administration_interface.hpp>
#include <eos/chain/chain_initializer_interface.hpp>
#include <eos/chain/exceptions.hpp>
#include <eos/chain/message_handling_contexts.hpp>
#include <eos/chain/protocol.hpp>

#include <chainbase/chainbase.hpp>
#include <fc/scoped_exit.hpp>
#include <boost/signals2/signal.hpp>
#include <fc/log/logger.hpp>

#include "pyobject.hpp"

using namespace chainbase;
using namespace fc;
using namespace eos::chain;

namespace python {
namespace database {

PyObject* database_create(string& path) {
   auto _db = new chainbase::database(fc::path(path), chainbase::database::read_write, 1 * 1024 * 1024);
   _db->add_index<account_index>();
   _db->add_index<permission_index>();
   _db->add_index<permission_link_index>();
   _db->add_index<action_permission_index>();
   _db->add_index<key_value_index>();
   _db->add_index<keystr_value_index>();
   _db->add_index<key128x128_value_index>();
   _db->add_index<key64x64x64_value_index>();

   _db->add_index<global_property_multi_index>();
   _db->add_index<dynamic_global_property_multi_index>();
   _db->add_index<block_summary_multi_index>();
   _db->add_index<transaction_multi_index>();
   _db->add_index<generated_transaction_multi_index>();
   _db->add_index<producer_multi_index>();

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

   const auto& recipient = _db.get<account_object,by_name>(Name(code));
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
