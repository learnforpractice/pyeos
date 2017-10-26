#include "database_.hpp"

using namespace eos::chain;
using namespace fc;
namespace bfs = boost::filesystem;

void test_all_() {
   eos::chain::testing_fixture testing;
   testing.test_all();
}

void test1() {
   auto db = chainbase::database(fc::path("tempdir"), chainbase::database::read_write, 1*1024*1024);
   try {
      db.add_index<account_index>();
      // Create an account
      db.create<account_object>([](account_object& a) {
          a.name = "billy";
      });

      // Make sure we can retrieve that account by name
      auto ptr = db.find<account_object, by_name, std::string>("billy");
      BOOST_CHECK(ptr != nullptr);
      db.flush();

   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }

}

void test2() {
   try {
      ilog("1");
      auto db = chainbase::database(fc::path("tempdir"), chainbase::database::read_only, 1*1024*1024);
      db.add_index<account_index>();
      // Make sure we can retrieve that account by name
      ilog("2");
      auto ptr = db.find<account_object, by_name, std::string>("billy");
      ilog("3");
      BOOST_CHECK(ptr != nullptr);
      ilog("${name}",("name",ptr->name));
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }

}


void test_db() {
   test1();
   test2();
}

void test_db_() {
   ilog("");
   auto db = chainbase::database(fc::path("tempdir"), chainbase::database::read_write, 1*1024*1024);
   db.add_index<account_index>();
   ilog("");
   try {
      // Create an account
      db.create<account_object>([](account_object& a) {
          a.name = "billy";
      });
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }

   ilog("");
   // Make sure we can retrieve that account by name
   auto ptr = db.find<account_object, by_name, std::string>("billy");
   BOOST_CHECK(ptr != nullptr);
   db.flush();
//      db.~database();
   ilog("");
   db = chainbase::database(fc::path("tempdir"), chainbase::database::read_write, 1*1024*1024);
   db.add_index<account_index>();
   ilog("");
   // Make sure we can retrieve that account by name
   ptr = db.find<account_object, by_name, std::string>("billy");
   BOOST_CHECK(ptr != nullptr);
   ilog("");
   db.flush();
//      db.~database();
}



void test_db2_() {
   auto db = chainbase::database(fc::path("tempdir"), chainbase::database::read_write, 1*1024*1024);
   db.add_index<account_index>();
   auto ses = db.start_undo_session(true);

   // Create an account
   db.create<account_object>([](account_object& a) {
       a.name = "billy";
   });

   // Make sure we can retrieve that account by name
   auto ptr = db.find<account_object, by_name, std::string>("billy");
   BOOST_CHECK(ptr != nullptr);

   // Undo creation of the account
   ses.undo();

   // Make sure we can no longer find the account
   ptr = db.find<account_object, by_name, std::string>("billy");
   BOOST_CHECK(ptr == nullptr);
}

