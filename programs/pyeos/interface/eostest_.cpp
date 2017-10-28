#include "eostest_.hpp"

using namespace std;
using namespace eos;
using namespace eos::chain;
using namespace eos::utilities;
using chainbase::database;

namespace bio = boost::iostreams;

#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>
#include <boost/signals2/shared_connection_block.hpp>

#include <eos/chain/account_object.hpp>
#include <eos/chain/producer_object.hpp>
#include <eos/chain/authority_checker.hpp>

#include <eos/utilities/tempdir.hpp>

#include <eos/native_contract/native_contract_chain_initializer.hpp>
#include <eos/native_contract/native_contract_chain_administrator.hpp>
#include <eos/native_contract/objects.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/smart_ref_impl.hpp>

#include <boost/range/adaptor/map.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

//#include "database_fixture.hpp"

uint32_t EOS_TESTING_GENESIS_TIMESTAMP = 1431700005;

namespace eos { namespace chain {
   using namespace native::eos;
//   using namespace native;

testing_fixture::testing_fixture() {
   default_genesis_state.initial_timestamp = fc::time_point_sec(EOS_TESTING_GENESIS_TIMESTAMP);
   for (int i = 0; i < config::BlocksPerRound; ++i) {
      auto name = std::string("inita"); name.back()+=i;
      auto private_key = fc::ecc::private_key::regenerate(fc::sha256::hash(name));
      public_key_type public_key = private_key.get_public_key();
      default_genesis_state.initial_accounts.emplace_back(name, 0, 100000, public_key, public_key);
      store_private_key(private_key);

      private_key = fc::ecc::private_key::regenerate(fc::sha256::hash(name + ".producer"));
      public_key = private_key.get_public_key();
      default_genesis_state.initial_producers.emplace_back(name, public_key);
      store_private_key(private_key);
   }

//   chain_db = new chainbase::database(get_temp_dir(), chainbase::database::read_write, TEST_DB_SIZE);
   chain_db = new chainbase::database(get_temp_dir(), chainbase::database::read_write, TEST_DB_SIZE);
   chain_log = new block_log(get_temp_dir() / "blocklog");
   chain_fdb = new fork_database();
   chain_initializer = new native_contract::native_contract_chain_initializer(genesis_state());
   chain = new testing_blockchain(*chain_db, *chain_fdb, *chain_log, *chain_initializer, *this);

}

fc::path testing_fixture::get_temp_dir(std::string id) {
   return fc::path("tempdir");
   if (id.empty()) {
      anonymous_temp_dirs.emplace_back();
      return anonymous_temp_dirs.back().path();
   }
   if (named_temp_dirs.count(id))
      return named_temp_dirs[id].path();
   return named_temp_dirs.emplace(std::make_pair(id, fc::temp_directory())).first->second.path();
}

const native_contract::genesis_state_type& testing_fixture::genesis_state() const {
   return default_genesis_state;
}

native_contract::genesis_state_type& testing_fixture::genesis_state() {
   return default_genesis_state;
}

void testing_fixture::store_private_key(const private_key_type& key) {
   key_ring[key.get_public_key()] = key;
}

private_key_type testing_fixture::get_private_key(const public_key_type& public_key) const {
   auto itr = key_ring.find(public_key);
   EOS_ASSERT(itr != key_ring.end(), missing_key_exception,
              "Private key corresponding to public key ${k} not known.", ("k", public_key));
   return itr->second;
}

flat_set<public_key_type> testing_fixture::available_keys() const {
   auto range = key_ring | boost::adaptors::map_keys;
   return {range.begin(), range.end()};
}

testing_blockchain::testing_blockchain(chainbase::database& db, fork_database& fork_db, block_log& blocklog,
                                   chain_initializer_interface& initializer, testing_fixture& fixture)
   : chain_controller(db, fork_db, blocklog, initializer, native_contract::make_administrator()),
     db(db),
     fixture(fixture) {}

void testing_blockchain::produce_blocks(uint32_t count, uint32_t blocks_to_miss) {
   if (count == 0)
      return;

   for (int i = 0; i < count; ++i) {
      auto slot = blocks_to_miss + 1;
      auto producer = get_producer(get_scheduled_producer(slot));
      auto private_key = fixture.get_private_key(producer.signing_key);
      generate_block(get_slot_time(slot), producer.owner, private_key, block_schedule::in_single_thread,
                     skip_trx_sigs? chain_controller::skip_transaction_signatures : 0);
   }
}

void testing_blockchain::sync_with(testing_blockchain& other) {
   // Already in sync?
   if (head_block_id() == other.head_block_id())
      return;
   // If other has a longer chain than we do, sync it to us first
   if (head_block_num() < other.head_block_num())
      return other.sync_with(*this);

   auto sync_dbs = [](testing_blockchain& a, testing_blockchain& b) {
      for (int i = 1; i <= a.head_block_num(); ++i) {
         auto block = a.fetch_block_by_number(i);
         if (block && !b.is_known_block(block->id())) {
            b.push_block(*block);
         }
      }
   };

   sync_dbs(*this, other);
   sync_dbs(other, *this);
}

types::Asset testing_blockchain::get_liquid_balance(const types::AccountName& account) {
   return get_database().get<BalanceObject, native::eos::byOwnerName>(account).balance;
}

types::Asset testing_blockchain::get_staked_balance(const types::AccountName& account) {
   return get_database().get<StakedBalanceObject, native::eos::byOwnerName>(account).stakedBalance;
}

types::Asset testing_blockchain::get_unstaking_balance(const types::AccountName& account) {
   return get_database().get<StakedBalanceObject, native::eos::byOwnerName>(account).unstakingBalance;
}

std::set<types::AccountName> testing_blockchain::get_approved_producers(const types::AccountName& account) {
   const auto& sbo = get_database().get<StakedBalanceObject, byOwnerName>(account);
   if (sbo.producerVotes.contains<ProducerSlate>()) {
      auto range = sbo.producerVotes.get<ProducerSlate>().range();
      return {range.begin(), range.end()};
   }
   return {};
}

types::PublicKey testing_blockchain::get_block_signing_key(const types::AccountName& producerName) {
   return get_database().get<producer_object, by_owner>(producerName).signing_key;
}

void testing_blockchain::sign_transaction(SignedTransaction& trx) const {
   auto keys = get_required_keys(trx, fixture.available_keys());
   for (const auto& k : keys) {
      // TODO: Use a real chain_id here
      trx.sign(fixture.get_private_key(k), chain_id_type{});
   }
}

fc::optional<ProcessedTransaction> testing_blockchain::push_transaction(SignedTransaction trx, uint32_t skip_flags) {
   if (skip_trx_sigs)
      skip_flags |= chain_controller::skip_transaction_signatures;

   if (auto_sign_trxs) {
      sign_transaction(trx);
   }

   if (hold_for_review) {
      review_storage = std::make_pair(trx, skip_flags);
      return {};
   }
   return chain_controller::push_transaction(trx, skip_flags);
}

void testing_fixture::create_account() {
   ilog("test_all");
   testing_blockchain& tb = *chain;
   ilog("chain_db->head_block_num(): ${n}",("n",tb.head_block_num()));

   string account_name("acc1");

   auto acc1_private_key = private_key_type::regenerate(fc::digest("acc1_private_key"));
   store_private_key(acc1_private_key);

   PublicKey acc1_public_key = acc1_private_key.get_public_key();
   auto active_key = Key_Authority(acc1_public_key);
   auto owner_key = Key_Authority(acc1_public_key);
   auto authority = Authority{1, {}, {{{"inita", "active"}, 1}}};
   {
         eos::chain::SignedTransaction trx;
         trx.scope = sort_names({ "inita", config::EosContractName });
         transaction_emplace_message(trx, config::EosContractName,
                            vector<types::AccountPermission>{{"inita", "active"}},
                            "newaccount", types::newaccount{"inita", account_name, owner_key, active_key, authority, Asset(100)});
         trx.expiration = tb.head_block_time() + 100;
         transaction_set_reference_block(trx, tb.head_block_id());
         tb.push_transaction(trx);
   }
}

void testing_fixture::create_account_manually(string& name) {
   testing_blockchain& tb = *chain;

   try {
      auto d = (chainbase::database*)&tb.get_database();
      auto& db = *d;
      db.create<account_object>([&name](account_object& a) {
          a.name = name;
      });

      db.create<BalanceObject>([&name](BalanceObject& b) {
         b.ownerName = name;
         b.balance = 111; //create.deposit.amount; TODO: make sure we credit this in @staked
      });

      // Make sure we can retrieve that account by name
      auto ptr = db.find<account_object, by_name, std::string>(name.c_str());
      BOOST_CHECK(ptr != nullptr);
      db.flush();

   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
}

void testing_fixture::get_account(string& account_name) {
   testing_blockchain& tb = *chain;

   try {
      const auto& d = tb.get_database();
      const auto& accnt = d.get<account_object, by_name>(account_name);
      const auto& balance = d.get<BalanceObject, byOwnerName>(account_name);
      ilog("${name},\t\t${balance}",("name",balance.ownerName)("balance",balance.balance));
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
}


void testing_fixture::produce_block() {
   testing_blockchain& tb = *chain;
   tb.produce_blocks(1);
}

void testing_fixture::test_all() {
   ilog("test_all");
   testing_blockchain& tb = *chain;
   tb.produce_blocks(1);
   ilog("chain_db->head_block_num(): ${n}",("n",tb.head_block_num()));
   Make_Account(tb, testapi);

   string account_name("acc1");
   try {
      const auto& d = tb.get_database();
      const auto& accnt = d.get<account_object, by_name>(account_name);
      const auto& balance = d.get<BalanceObject, byOwnerName>(account_name);
      ilog("%{balance}",("balance",balance.balance));
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }

   auto acc1_private_key = private_key_type::regenerate(fc::digest("acc1_private_key"));
   store_private_key(acc1_private_key);

   PublicKey acc1_public_key = acc1_private_key.get_public_key();
   auto active_key = Key_Authority(acc1_public_key);
   auto owner_key = Key_Authority(acc1_public_key);
   auto authority = Authority{1, {}, {{{"inita", "active"}, 1}}};

//   MKACCT_IMPL(tb, acc1, inita, active_key, owner_key, authority, Asset(100))

   {
         eos::chain::SignedTransaction trx;
         trx.scope = sort_names({ "inita", config::EosContractName });
         transaction_emplace_message(trx, config::EosContractName,
                            vector<types::AccountPermission>{{"inita", "active"}},
                            "newaccount", types::newaccount{"inita", account_name, owner_key, active_key, authority, Asset(100)});
         trx.expiration = tb.head_block_time() + 100;
         transaction_set_reference_block(trx, tb.head_block_id());
         tb.push_transaction(trx);
   }

   tb.produce_blocks(1);

   ilog("chain_db->head_block_num(): ${n}",("n",tb.head_block_num()));

   try {
      const auto& d = tb.get_database();
      const auto& accnt = d.get<account_object, by_name>(account_name);
      const auto& balance = d.get<BalanceObject, byOwnerName>(account_name);
      ilog("${name},\t\t ${balance}",("name",balance.ownerName)("balance",balance.balance));
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   chain_db->flush();
}

testing_fixture::~testing_fixture() {
   delete chain;
   delete chain_db;
   delete chain_log;
   delete chain_fdb;
   delete chain_initializer;
   ilog("testing_fixture::~testing_fixture()");
}
}}



using namespace eos::chain;
using namespace fc;
namespace bfs = boost::filesystem;

static testing_fixture *testing = 0;


namespace python {
namespace test {


void test_start_() {
   ilog("");
   if (testing) {
      return;
   }
   try {
      testing = new testing_fixture();
   } catch (fc::assert_exception& e) {
      elog(e.to_detail_string());
   } catch (fc::exception& e) {
      elog(e.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }

}

void test_create_account_() {
   ilog("");
   assert(testing);
   testing->create_account();
}

void create_account_manually_(string& name) {
   ilog("");
   assert(testing);
   testing->create_account_manually(name);
}

void get_account_(string& name) {
   ilog("");
   assert(testing);
   testing->get_account(name);
}

void get_block_num_() {
   ilog("chain_db->head_block_num(): ${n}",("n",testing->chain->head_block_num()));
   ilog("chain_db->last_irreversible_block_num(): ${n}",("n",testing->chain->last_irreversible_block_num()));
}

void get_block_log_num_() {
   assert(testing);
   auto head = testing->chain_log->read_head();
   if (head) {
      auto n = head->block_num();
      ilog("chain_log.read_head()->block_num(): ${n}",("n",n));
   } else {
      ilog("testing->chain_log->read_head() is null");
   }

}

void produce_block_() {
   ilog("");
   assert(testing);
   testing->produce_block();
}

void test_end_() {
   ilog("");
   if (testing) {
      delete testing;
      testing = 0;
   }
}



void test_all_() {
   eos::chain::testing_fixture testing;
   testing.test_all();
}

void test_create_account_(string& name) {
   auto db = chainbase::database(fc::path("tempdir"), chainbase::database::read_write, 1*1024*1024);
   try {
      db.add_index<account_index>();
      // Create an account
      db.create<account_object>([&name](account_object& a) {
          a.name = name;
      });

      // Make sure we can retrieve that account by name
      auto ptr = db.find<account_object, by_name, std::string>(name.c_str());
      BOOST_CHECK(ptr != nullptr);
      db.flush();

   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }

}

void test_get_account_(string& name) {
   try {
      ilog("1");
      auto db = chainbase::database(fc::path("tempdir"), chainbase::database::read_only, 1*1024*1024);
      db.add_index<account_index>();
      // Make sure we can retrieve that account by name
      ilog("2");
      auto ptr = db.find<account_object, by_name, std::string>(name.c_str());
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
   string name("bob");
   test_create_account_(name);
   test_get_account_(name);
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

}
}
