/*
 * eostest_.hpp
 *
 *  Created on: Oct 25, 2017
 *      Author: newworld
 */

#ifndef PROGRAMS_PYEOS_INTERFACE_EOSTEST__HPP_
#define PROGRAMS_PYEOS_INTERFACE_EOSTEST__HPP_



#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <sstream>
#include <vector>

#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/test/unit_test.hpp>

#include <eosio/chain/account_object.hpp>
#include <eosio/chain/block_summary_object.hpp>
#include <eosio/chain/chain_controller.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/wasm_interface.hpp>

#include <eosio/utilities/tempdir.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/exception/exception.hpp>

//#include "../common/database_fixture.hpp"

#include <IR/Module.h>
#include <IR/Validate.h>
#include <Inline/BasicTypes.h>
#include <Runtime/Runtime.h>
#include <WASM/WASM.h>
#include <WAST/WAST.h>


#include <appbase/application.hpp>
#include <chainbase/chainbase.hpp>
#include <fc/exception/exception.hpp>


////////////////////////////////////////

#include <eosio/chain/chain_controller.hpp>
#include <eosio/chain/producer_object.hpp>
#include <eosio/chain/exceptions.hpp>


#include <eosio/utilities/tempdir.hpp>

#include <fc/io/json.hpp>
#include <fc/smart_ref_impl.hpp>

#include <boost/range/algorithm/sort.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/facilities/overload.hpp>

#include <iostream>

using namespace eosio::chain;

extern uint32_t EOS_TESTING_GENESIS_TIMESTAMP;

#define HAVE_DATABASE_FIXTURE
//#include "testing_macros.hpp"

#include <eos/chain/wast_to_wasm.hpp>

//#include "macro_support.hpp"


#include <fc/crypto/digest.hpp>

/**
 * @file Contains support macros for the testcase helper macros. These macros are implementation details, and thus
 * should not be used directly. Use their frontends instead.
 */

#define MKCHAIN1(name) \
   chainbase::database name ## _db(get_temp_dir(), chainbase::database::read_write, TEST_DB_SIZE); \
   block_log name ## _log(get_temp_dir() / "blocklog"); \
   fork_database name ## _fdb; \
   native_contract::native_contract_chain_initializer name ## _initializer(genesis_state()); \
   testing_blockchain name(name ## _db, name ## _fdb, name ## _log, name ## _initializer, *this); \
   BOOST_TEST_CHECKPOINT("Created blockchain " << #name);
#define MKCHAIN2(name, id) \
   chainbase::database name ## _db(get_temp_dir(#id), chainbase::database::read_write, TEST_DB_SIZE); \
   block_log name ## _log(get_temp_dir(#id) / "blocklog"); \
   fork_database name ## _fdb; \
   native_contract::native_contract_chain_initializer name ## _initializer(genesis_state()); \
   testing_blockchain name(name ## _db, name ## _fdb, name ## _log, name ## _initializer, *this); \
   BOOST_TEST_CHECKPOINT("Created blockchain " << #name);
#define MKCHAINS_MACRO(x, y, name) Make_Blockchain(name)

#define MKNET1(name) testing_network name; BOOST_TEST_CHECKPOINT("Created testnet " << #name);
#define MKNET2_MACRO(x, name, chain) name.connect_blockchain(chain);
#define MKNET2(name, ...) MKNET1(name) BOOST_PP_SEQ_FOR_EACH(MKNET2_MACRO, name, __VA_ARGS__)

inline std::vector<Name> sort_names( std::vector<Name>&& names ) {
   std::sort( names.begin(), names.end() );
   auto itr = std::unique( names.begin(), names.end() );
   names.erase( itr, names.end() );
   return names;
}

#define Complex_Authority_macro_Key(r, data, key_bubble) \
   data.keys.emplace_back(BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 0, key_bubble), _public_key), \
                          BOOST_PP_TUPLE_ELEM(2, 1, key_bubble));
#define Complex_Authority_macro_Account(r, data, account_bubble) \
   data.accounts.emplace_back(types::AccountPermission{BOOST_PP_TUPLE_ELEM(3, 0, account_bubble), \
                                                       BOOST_PP_TUPLE_ELEM(3, 1, account_bubble)}, \
                              BOOST_PP_TUPLE_ELEM(3, 2, account_bubble));

#define MKACCT_IMPL(chain_, name, creator, active, owner, recovery, deposit) \
   { \
      eosio::chain::SignedTransaction trx; \
      trx.scope = sort_names({ #creator, config::EosContractName }); \
      transaction_emplace_message(trx, config::EosContractName, \
                         vector<types::AccountPermission>{{#creator, "active"}}, \
                         "newaccount", types::newaccount{#creator, #name, owner, active, recovery, deposit}); \
      trx.expiration = chain_.head_block_time() + 100; \
      transaction_set_reference_block(trx, chain_.head_block_id()); \
      chain_.push_transaction(trx); \
      BOOST_TEST_CHECKPOINT("Created account " << #name); \
   }
#define MKACCT2(chain, name) \
   Make_Key(name) \
   MKACCT_IMPL(chain, name, inita, Key_Authority(name ## _public_key), Key_Authority(name ## _public_key), \
               Account_Authority(inita), Asset(100))
#define MKACCT3(chain, name, creator) \
   Make_Key(name) \
   MKACCT_IMPL(chain, name, creator, Key_Authority(name ## _public_key), Key_Authority(name ## _public_key), \
               Account_Authority(creator), Asset(100))
#define MKACCT4(chain, name, creator, deposit) \
   Make_Key(name) \
   MKACCT_IMPL(chain, name, creator, Key_Authority(name ## _public_key), Key_Authority(name ## _public_key), \
               Account_Authority(creator), deposit)
#define MKACCT5(chain, name, creator, deposit, owner) \
   Make_Key(name) \
   MKACCT_IMPL(chain, name, creator, owner, Key_Authority(name ## _public_key), Account_Authority(creator), deposit)
#define MKACCT6(chain, name, creator, deposit, owner, active) \
   MKACCT_IMPL(chain, name, creator, owner, active, Account_Authority(creator), deposit)
#define MKACCT7(chain, name, creator, deposit, owner, active, recovery) \
   MKACCT_IMPL(chain, name, creator, owner, active, recovery, deposit)

#define SETCODE3(chain, acct, wast) \
   { \
      auto wasm = eosio::chain::wast_to_wasm(wast); \
      types::setcode handler; \
      handler.account = #acct; \
      handler.code.assign(wasm.begin(), wasm.end()); \
      eosio::chain::SignedTransaction trx; \
      trx.scope = sort_names({config::EosContractName, #acct}); \
      transaction_emplace_message(trx, config::EosContractName, vector<types::AccountPermission>{{#acct,"active"}}, \
                                  "setcode", handler); \
   }

#define SETAUTH5(chain, account, authname, parentname, auth) \
   { \
      eosio::chain::SignedTransaction trx; \
      trx.scope = {#account}; \
      transaction_emplace_message(trx, config::EosContractName, \
                         vector<types::AccountPermission>{{#account,"active"}}, \
                         "updateauth", types::updateauth{#account, authname, parentname, auth}); \
      trx.expiration = chain.head_block_time() + 100; \
      transaction_set_reference_block(trx, chain.head_block_id()); \
      chain.push_transaction(trx); \
      BOOST_TEST_CHECKPOINT("Set " << #account << "'s " << authname << " authority."); \
   }

#define DELAUTH3(chain, account, authname) \
   { \
      eosio::chain::SignedTransaction trx; \
      trx.scope = {#account}; \
      transaction_emplace_message(trx, config::EosContractName, \
                         vector<types::AccountPermission>{{#account,"active"}}, \
                         "deleteauth", types::deleteauth{#account, authname}); \
      trx.expiration = chain.head_block_time() + 100; \
      transaction_set_reference_block(trx, chain.head_block_id()); \
      chain.push_transaction(trx); \
      BOOST_TEST_CHECKPOINT("Deleted " << #account << "'s " << authname << " authority."); \
   }

#define LINKAUTH5(chain, account, authname, codeacct, messagetype) \
   { \
      eosio::chain::SignedTransaction trx; \
      trx.scope = {#account}; \
      transaction_emplace_message(trx, config::EosContractName, \
                         vector<types::AccountPermission>{{#account,"active"}}, \
                         "linkauth", types::linkauth{#account, #codeacct, messagetype, authname}); \
      trx.expiration = chain.head_block_time() + 100; \
      transaction_set_reference_block(trx, chain.head_block_id()); \
      chain.push_transaction(trx); \
      BOOST_TEST_CHECKPOINT("Link " << #codeacct << "::" << messagetype << " to " << #account \
                            << "'s " << authname << " authority."); \
   }
#define LINKAUTH4(chain, account, authname, codeacct) LINKAUTH5(chain, account, authname, codeacct, "")

#define UNLINKAUTH4(chain, account, codeacct, messagetype) \
   { \
      eosio::chain::SignedTransaction trx; \
      trx.scope = {#account}; \
      transaction_emplace_message(trx, config::EosContractName, \
                         vector<types::AccountPermission>{{#account,"active"}}, \
                         "unlinkauth", types::unlinkauth{#account, #codeacct, messagetype}); \
      trx.expiration = chain.head_block_time() + 100; \
      transaction_set_reference_block(trx, chain.head_block_id()); \
      chain.push_transaction(trx); \
      BOOST_TEST_CHECKPOINT("Unlink " << #codeacct << "::" << messagetype << " from " << #account); \
   }
#define LINKAUTH3(chain, account, codeacct) LINKAUTH5(chain, account, codeacct, "")

#define XFER5(chain, sender, recipient, Amount, memo) \
   { \
      eosio::chain::SignedTransaction trx; \
      trx.scope = sort_names({#sender,#recipient}); \
      transaction_emplace_message(trx, config::EosContractName, \
                         vector<types::AccountPermission>{ {#sender,"active"} }, \
                         "transfer", types::transfer{#sender, #recipient, Amount.amount, memo}); \
      trx.expiration = chain.head_block_time() + 100; \
      transaction_set_reference_block(trx, chain.head_block_id()); \
      chain.push_transaction(trx); \
      BOOST_TEST_CHECKPOINT("Transferred " << Amount << " from " << #sender << " to " << #recipient); \
   }
#define XFER4(chain, sender, recipient, amount) XFER5(chain, sender, recipient, amount, "")

#define STAKE4(chain, sender, recipient, amount) \
   { \
      eosio::chain::SignedTransaction trx; \
      trx.scope = sort_names( { #sender, #recipient, config::EosContractName } ); \
      transaction_emplace_message(trx, config::EosContractName, vector<types::AccountPermission>{{#sender, "active"}}, \
                        "lock", types::lock{#sender, #recipient, amount}); \
      trx.expiration = chain.head_block_time() + 100; \
      transaction_set_reference_block(trx, chain.head_block_id()); \
      chain.push_transaction(trx); \
      BOOST_TEST_CHECKPOINT("Staked " << amount << " to " << #recipient); \
   }
#define STAKE3(chain, account, amount) STAKE4(chain, account, account, amount)

#define BEGIN_UNSTAKE3(chain, account, amount) \
   { \
      eosio::chain::SignedTransaction trx; \
      trx.scope = sort_names( { config::EosContractName } ); \
      transaction_emplace_message(trx, config::EosContractName, \
                         vector<types::AccountPermission>{{#account, "active"}}, \
                         "unlock", types::unlock{#account, amount}); \
      trx.expiration = chain.head_block_time() + 100; \
      transaction_set_reference_block(trx, chain.head_block_id()); \
      chain.push_transaction(trx); \
      BOOST_TEST_CHECKPOINT("Begin unstake " << amount << " to " << #account); \
   }

#define FINISH_UNSTAKE3(chain, account, amount) \
   { \
      eosio::chain::SignedTransaction trx; \
      trx.scope = sort_names( { config::EosContractName, #account } ); \
      transaction_emplace_message(trx, config::EosContractName, vector<types::AccountPermission>{{#account, "active"}}, \
                         "claim", types::claim{#account, amount}); \
      trx.expiration = chain.head_block_time() + 100; \
      transaction_set_reference_block(trx, chain.head_block_id()); \
      chain.push_transaction(trx); \
      BOOST_TEST_CHECKPOINT("Finish unstake " << amount << " to " << #account); \
   }

#define MKPDCR4(chain, owner, key, cfg) \
   { \
      eosio::chain::SignedTransaction trx; \
      trx.scope = sort_names( {#owner, config::EosContractName} ); \
      transaction_emplace_message(trx, config::EosContractName, \
                         vector<types::AccountPermission>{{#owner, "active"}}, \
                         "setproducer", types::setproducer{#owner, key, cfg}); \
      trx.expiration = chain.head_block_time() + 100; \
      transaction_set_reference_block(trx, chain.head_block_id()); \
      chain.push_transaction(trx); \
      BOOST_TEST_CHECKPOINT("Create producer " << #owner); \
   }
#define MKPDCR3(chain, owner, key) MKPDCR4(chain, owner, key, BlockchainConfiguration{});
#define MKPDCR2(chain, owner) \
   Make_Key(owner ## _producer); \
   MKPDCR4(chain, owner, owner ## _producer_public_key, BlockchainConfiguration{});

#define APPDCR4(chain, voter, producer, approved) \
   { \
      eosio::chain::SignedTransaction trx; \
      trx.scope = sort_names( {#voter, config::EosContractName} ); \
      transaction_emplace_message(trx, config::EosContractName,  \
                         vector<types::AccountPermission>{{#voter, "active"}}, \
                         "okproducer", types::okproducer{#voter, #producer, approved? 1 : 0}); \
      trx.expiration = chain.head_block_time() + 100; \
      transaction_set_reference_block(trx, chain.head_block_id()); \
      chain.push_transaction(trx); \
      BOOST_TEST_CHECKPOINT("Set producer approval from " << #voter << " for " << #producer << " to " << approved); \
   }

#define UPPDCR4(chain, owner, key, cfg) \
   { \
      eosio::chain::SignedTransaction trx; \
      trx.scope = sort_names( {owner, config::EosContractName} ); \
      transaction_emplace_message(trx, config::EosContractName,  \
                         vector<types::AccountPermission>{{owner, "active"}}, \
                         "setproducer", types::setproducer{owner, key, cfg}); \
      trx.expiration = chain.head_block_time() + 100; \
      transaction_set_reference_block(trx, chain.head_block_id()); \
      chain.push_transaction(trx); \
      BOOST_TEST_CHECKPOINT("Update producer " << owner); \
   }
#define UPPDCR3(chain, owner, key) UPPDCR4(chain, owner, key, chain.get_producer(owner).configuration)

//macro_support.hpp





/// Some helpful macros to reduce boilerplate when making testcases
/// @{

/**
 * @brief Create/Open a testing_blockchain, optionally with an ID
 *
 * Creates and opens a testing_blockchain with the first argument as its name, and, if present, the second argument as
 * its ID. The ID should be provided without quotes.
 *
 * Example:
 * @code{.cpp}
 * // Create testing_blockchain chain1
 * Make_Blockchain(chain1)
 *
 * // The above creates the following objects:
 * chainbase::database chain1_db;
 * block_log chain1_log;
 * fork_database chain1_fdb;
 * native_contract::native_contract_chain_initializer chain1_initializer;
 * testing_blockchain chain1;
 * @endcode
 */
#define Make_Blockchain(...) BOOST_PP_OVERLOAD(MKCHAIN, __VA_ARGS__)(__VA_ARGS__)
/**
 * @brief Similar to @ref Make_Blockchain, but works with several chains at once
 *
 * Creates and opens several testing_blockchains
 *
 * Example:
 * @code{.cpp}
 * // Create testing_blockchains chain1 and chain2, with chain2 having ID "id2"
 * Make_Blockchains((chain1)(chain2, id2))
 * @endcode
 */
#define Make_Blockchains(...) BOOST_PP_SEQ_FOR_EACH(MKCHAINS_MACRO, _, __VA_ARGS__)

/**
 * @brief Make_Network is a shorthand way to create a testing_network and connect some testing_blockchains to it.
 *
 * Example usage:
 * @code{.cpp}
 * // Create and open testing_blockchains named alice, bob, and charlie
 * MKDBS((alice)(bob)(charlie))
 * // Create a testing_network named net and connect alice and bob to it
 * Make_Network(net, (alice)(bob))
 *
 * // Connect charlie to net, then disconnect alice
 * net.connect_blockchain(charlie);
 * net.disconnect_blockchain(alice);
 *
 * // Create a testing_network named net2 with no blockchains connected
 * Make_Network(net2)
 * @endcode
 */
#define Make_Network(...) BOOST_PP_OVERLOAD(MKNET, __VA_ARGS__)(__VA_ARGS__)

/**
 * @brief Make_Key is a shorthand way to create a keypair
 *
 * @code{.cpp}
 * // This line:
 * Make_Key(a_key)
 * // ...defines these objects:
 * private_key_type a_key_private_key;
 * PublicKey a_key_public_key;
 * // The private key is generated off of the sha256 hash of "a_key_private_key", so it should be unique from all
 * // other keys created with Make_Key in the same scope.
 * @endcode
 */
#ifdef HAVE_DATABASE_FIXTURE
#define Make_Key(name) auto name ## _private_key = private_key_type::regenerate(fc::digest(#name "_private_key")); \
   store_private_key(name ## _private_key); \
   PublicKey name ## _public_key = name ## _private_key.get_public_key(); \
   BOOST_TEST_CHECKPOINT("Created key " #name "_public_key");
#else
#define Make_Key(name) auto name ## _private_key = private_key_type::regenerate(fc::digest(#name "_private_key")); \
   PublicKey name ## _public_key = name ## _private_key.get_public_key(); \
   BOOST_TEST_CHECKPOINT("Created key " #name "_public_key");
#endif

/**
 * @brief Key_Authority is a shorthand way to create an inline Authority based on a key
 *
 * Invoke Key_Authority passing the name of a public key in the current scope, and Key_Authority will resolve inline to
 * an authority which can be satisfied by a signature generated by the corresponding private key.
 */
#define Key_Authority(pubkey) (Authority{1, {{pubkey, 1}}, {}})
/**
 * @brief Account_Authority is a shorthand way to create an inline Authority based on an account
 *
 * Invoke Account_Authority passing the name of an account, and Account_Authority will resolve inline to an authority
 * which can be satisfied by the provided account's active authority.
 */
#define Account_Authority(account) (Authority{1, {}, {{{#account, "active"}, 1}}})
/**
 * @brief Complex_Authority is a shorthand way to create an arbitrary inline @ref Authority
 *
 * Invoke Complex_Authority passing the weight threshold necessary to satisfy the authority, a bubble list of keys and
 * weights, and a bubble list of accounts and weights.
 *
 * Key bubbles are structured as ((key_name, key_weight))
 * Account bubbles are structured as (("account_name", "account_authority", weight))
 *
 * Example:
 * @code{.cpp}
 * // Create an authority which can be satisfied with a master key, or with any three of:
 * // - key_1
 * // - key_2
 * // - key_3
 * // - Account alice's "test_multisig" authority
 * // - Account bob's "test_multisig" authority
 * Make_Key(master_key)
 * Make_Key(key_1)
 * Make_Key(key_2)
 * Make_Key(key_3)
 * auto auth = Complex_Authority(5, ((master_key, 5))((key_1, 2))((key_2, 2))((key_3, 2)),
 *                               (("alice", "test_multisig", 2))(("bob", "test_multisig", 2));
 * @endcode
 */
#define Complex_Authority(THRESHOLD, KEY_BUBBLES, ACCOUNT_BUBBLES) \
   [&]{ \
      Authority x; \
      x.threshold = THRESHOLD; \
      BOOST_PP_SEQ_FOR_EACH(Complex_Authority_macro_Key, x, KEY_BUBBLES) \
      BOOST_PP_SEQ_FOR_EACH(Complex_Authority_macro_Account, x, ACCOUNT_BUBBLES) \
      return x; \
   }()

/**
 * @brief Make_Account is a shorthand way to create an account
 *
 * Use Make_Account to create an account, including keys. The changes will be applied via a transaction applied to the
 * provided blockchain object. The changes will not be incorporated into a block; they will be left in the pending
 * state.
 *
 * Unless overridden, new accounts are created with a balance of Asset(100)
 *
 * Example:
 * @code{.cpp}
 * Make_Account(chain, joe)
 * // ... creates these objects:
 * private_key_type joe_private_key;
 * PublicKey joe_public_key;
 * // ...and also registers the account joe with owner and active authorities satisfied by these keys, created by
 * // init0, with init0's active authority as joe's recovery authority, and initially endowed with Asset(100)
 * @endcode
 *
 * You may specify a third argument for the creating account:
 * @code{.cpp}
 * // Same as MKACCT(chain, joe) except that sam will create joe's account instead of init0
 * Make_Account(chain, joe, sam)
 * @endcode
 *
 * You may specify a fourth argument for the amount to transfer in account creation:
 * @code{.cpp}
 * // Same as MKACCT(chain, joe, sam) except that sam will send joe ASSET(100) during creation
 * Make_Account(chain, joe, sam, Asset(100))
 * @endcode
 *
 * You may specify a fifth argument, which will be used as the owner authority (must be an Authority, NOT a key!).
 *
 * You may specify a sixth argument, which will be used as the active authority. If six or more arguments are provided,
 * the default keypair will NOT be created or put into scope.
 *
 * You may specify a seventh argument, which will be used as the recovery authority.
 */
#define Make_Account(...) BOOST_PP_OVERLOAD(MKACCT, __VA_ARGS__)(__VA_ARGS__)

/**
 * @brief Shorthand way to set the code for an account
 *
 * @code{.cpp}
 * char* wast = //...
 * Set_Code(chain, codeacct, wast);
 * @endcode
 */
#define Set_Code(...) BOOST_PP_OVERLOAD(SETCODE, __VA_ARGS__)(__VA_ARGS__)

/**
 * @brief Shorthand way to create or update named authority on an account
 *
 * @code{.cpp}
 * // Add a new authority named "money" to account "alice" as a child of her active authority
 * Authority newAuth = //...
 * Set_Authority(chain, alice, "money", "active", newAuth);
 * @endcode
 */
#define Set_Authority(...) BOOST_PP_OVERLOAD(SETAUTH, __VA_ARGS__)(__VA_ARGS__)
/**
 * @brief Shorthand way to delete named authority from an account
 *
 * @code{.cpp}
 * // Delete authority named "money" from account "alice"
 * Delete_Authority(chain, alice, "money");
 * @endcode
 */
#define Delete_Authority(...) BOOST_PP_OVERLOAD(DELAUTH, __VA_ARGS__)(__VA_ARGS__)
/**
 * @brief Shorthand way to link named authority with a contract/message type
 *
 * @code{.cpp}
 * // Link alice's "money" authority with eosio::transfer
 * Link_Authority(chain, alice, "money", eos, "transfer");
 * // Set alice's "native" authority as default for eos contract
 * Link_Authority(chain, alice, "money", eos);
 * @endcode
 */
#define Link_Authority(...) BOOST_PP_OVERLOAD(LINKAUTH, __VA_ARGS__)(__VA_ARGS__)
/**
 * @brief Shorthand way to unlink named authority from a contract/message type
 *
 * @code{.cpp}
 * // Unlink alice's authority for eosio::transfer
 * Unlink_Authority(chain, alice, eos, "transfer");
 * // Unset alice's default authority for eos contract
 * Unlink_Authority(chain, alice, eos);
 * @endcode
 */
#define Unlink_Authority(...) BOOST_PP_OVERLOAD(UNLINKAUTH, __VA_ARGS__)(__VA_ARGS__)

/**
 * @brief Shorthand way to transfer funds
 *
 * Use Transfer_Asset to send funds from one account to another:
 * @code{.cpp}
 * // Send 10 EOS from alice to bob
 * Transfer_Asset(chain, alice, bob, Asset(10));
 *
 * // Send 10 EOS from alice to bob with memo "Thanks for all the fish!"
 * Transfer_Asset(chain, alice, bob, Asset(10), "Thanks for all the fish!");
 * @endcode
 *
 * The changes will be applied via a transaction applied to the provided blockchain object. The changes will not be
 * incorporated into a block; they will be left in the pending state.
 */
#define Transfer_Asset(...) BOOST_PP_OVERLOAD(XFER, __VA_ARGS__)(__VA_ARGS__)

/**
 * @brief Shorthand way to convert liquid funds to staked funds
 *
 * Use Stake_Asset to stake liquid funds:
 * @code{.cpp}
 * // Convert 10 of bob's EOS from liquid to staked
 * Stake_Asset(chain, bob, Asset(10).amount);
 *
 * // Stake and transfer 10 EOS from alice to bob (alice pays liquid EOS and bob receives stake)
 * Stake_Asset(chain, alice, bob, Asset(10).amount);
 * @endcode
 */
#define Stake_Asset(...) BOOST_PP_OVERLOAD(STAKE, __VA_ARGS__)(__VA_ARGS__)

/**
 * @brief Shorthand way to begin conversion from staked funds to liquid funds
 *
 * Use Unstake_Asset to begin unstaking funds:
 * @code{.cpp}
 * // Begin unstaking 10 of bob's EOS
 * Unstake_Asset(chain, bob, Asset(10).amount);
 * @endcode
 *
 * This can also be used to cancel an unstaking in progress, by passing Asset(0) as the amount.
 */
#define Begin_Unstake_Asset(...) BOOST_PP_OVERLOAD(BEGIN_UNSTAKE, __VA_ARGS__)(__VA_ARGS__)

/**
 * @brief Shorthand way to claim unstaked EOS as liquid
 *
 * Use Finish_Unstake_Asset to liquidate unstaked funds:
 * @code{.cpp}
 * // Reclaim as liquid 10 of bob's unstaked EOS
 * Unstake_Asset(chain, bob, Asset(10).amount);
 * @endcode
 */
#define Finish_Unstake_Asset(...) BOOST_PP_OVERLOAD(FINISH_UNSTAKE, __VA_ARGS__)(__VA_ARGS__)


/**
 * @brief Shorthand way to set voting proxy
 *
 * Use Set_Proxy to set what account a stakeholding account proxies its voting power to
 * @code{.cpp}
 * // Proxy sam's votes to bob
 * Set_Proxy(chain, sam, bob);
 *
 * // Unproxy sam's votes
 * Set_Proxy(chain, sam, sam);
 * @endcode
 */
#define Set_Proxy(chain, stakeholder, proxy) \
{ \
   eosio::chain::SignedTransaction trx; \
   if (std::string(#stakeholder) != std::string(#proxy)) \
      transaction_emplace_message(trx, config::EosContractName, \
                         vector<types::AccountPermission>{ {#stakeholder,"active"} }, "setproxy", types::setproxy{#stakeholder, #proxy}); \
   else \
      transaction_emplace_message(trx, config::EosContractName, \
                         vector<types::AccountPermission>{ {#stakeholder,"active"} }, "setproxy", types::setproxy{#stakeholder, #proxy}); \
   trx.expiration = chain.head_block_time() + 100; \
   transaction_set_reference_block(trx, chain.head_block_id()); \
   chain.push_transaction(trx); \
}

/**
 * @brief Shorthand way to create a block producer
 *
 * Use Make_Producer to create a block producer:
 * @code{.cpp}
 * // Create a block producer belonging to joe using signing_key as the block signing key and config as the producer's
 * // vote for a @ref BlockchainConfiguration:
 * Make_Producer(chain, joe, signing_key, config);
 *
 * // Create a block producer belonging to joe using signing_key as the block signing key:
 * Make_Producer(chain, joe, signing_key);
 *
 * // Create a block producer belonging to joe, using a new key as the block signing key:
 * Make_Producer(chain, joe);
 * // ... creates the objects:
 * private_key_type joe_producer_private_key;
 * PublicKey joe_producer_public_key;
 * @endcode
 */
#define Make_Producer(...) BOOST_PP_OVERLOAD(MKPDCR, __VA_ARGS__)(__VA_ARGS__)

/**
 * @brief Shorthand way to set approval of a block producer
 *
 * Use Approve_Producer to change an account's approval of a block producer:
 * @code{.cpp}
 * // Set joe's approval for pete's block producer to Approve
 * Approve_Producer(chain, joe, pete, true);
 * // Set joe's approval for pete's block producer to Disapprove
 * Approve_Producer(chain, joe, pete, false);
 * @endcode
 */
#define Approve_Producer(...) BOOST_PP_OVERLOAD(APPDCR, __VA_ARGS__)(__VA_ARGS__)

/**
 * @brief Shorthand way to update a block producer
 *
 * @note Unlike with the Make_* macros, the Update_* macros take an expression as the owner/name field, so be sure to
 * wrap names like this in quotes. You may also pass a normal C++ expression to be evaulated here instead. The reason
 * for this discrepancy is that the Make_* macros add identifiers to the current scope based on the owner/name field;
 * moreover, which can't be done with C++ expressions; however, the Update_* macros do not add anything to the scope,
 * and it's more likely that these will be used in a loop or other context where it is inconvenient to know the
 * owner/name at compile time.
 *
 * Use Update_Producer to update a block producer:
 * @code{.cpp}
 * // Update a block producer belonging to joe using signing_key as the new block signing key, and config as the
 * // producer's new vote for a @ref BlockchainConfiguration:
 * Update_Producer(chain, "joe", signing_key, config)
 *
 * // Update a block producer belonging to joe using signing_key as the new block signing key:
 * Update_Producer(chain, "joe", signing_key)
 * @endcode
 */
#define Update_Producer(...) BOOST_PP_OVERLOAD(UPPDCR, __VA_ARGS__)(__VA_ARGS__)

/// @}

//testing_macros.hpp

#define TEST_DB_SIZE (1024*1024*1000)

#define EOS_REQUIRE_THROW( expr, exc_type )          \
{                                                         \
   std::string req_throw_info = fc::json::to_string(      \
      fc::mutable_variant_object()                        \
      ("source_file", __FILE__)                           \
      ("source_lineno", __LINE__)                         \
      ("expr", #expr)                                     \
      ("exc_type", #exc_type)                             \
      );                                                  \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "EOS_REQUIRE_THROW begin "             \
         << req_throw_info << std::endl;                  \
   BOOST_REQUIRE_THROW( expr, exc_type );                 \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "EOS_REQUIRE_THROW end "               \
         << req_throw_info << std::endl;                  \
}

#define EOS_CHECK_THROW( expr, exc_type )            \
{                                                         \
   std::string req_throw_info = fc::json::to_string(      \
      fc::mutable_variant_object()                        \
      ("source_file", __FILE__)                           \
      ("source_lineno", __LINE__)                         \
      ("expr", #expr)                                     \
      ("exc_type", #exc_type)                             \
      );                                                  \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "EOS_CHECK_THROW begin "               \
         << req_throw_info << std::endl;                  \
   BOOST_CHECK_THROW( expr, exc_type );                   \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "EOS_CHECK_THROW end "                 \
         << req_throw_info << std::endl;                  \
}

namespace eos { namespace chain {
FC_DECLARE_EXCEPTION(testing_exception, 6000000, "test framework exception")
FC_DECLARE_DERIVED_EXCEPTION(missing_key_exception, eosio::chain::testing_exception, 6010000, "key could not be found")

class testing_blockchain;

/**
 * @brief The testing_fixture class provides various services relevant to testing the blockchain.
 */
class testing_fixture {
public:
   testing_fixture();
   virtual ~testing_fixture();

   /**
    * @brief Get a temporary directory to store data in during this test
    *
    * @param id Identifier for the temporary directory. All requests for directories with the same ID will receive the
    * same path. If id is empty, a unique directory will be returned.
    *
    * @return Path to the temporary directory
    *
    * This method makes it easy to get temporary directories for the duration of a test. All returned directories will
    * be automatically deleted when the testing_fixture is destroyed.
    *
    * If multiple calls to this function are made with the same id, the same path will be returned. Multiple calls with
    * distinct ids will not return the same path. If called with an empty id, a unique path will be returned which will
    * not be returned from any subsequent call.
    */
   fc::path get_temp_dir(std::string id = std::string());

   const native_contract::genesis_state_type& genesis_state() const;
   native_contract::genesis_state_type& genesis_state();

   void store_private_key(const private_key_type& key);
   private_key_type get_private_key(const public_key_type& public_key) const;
   flat_set<public_key_type> available_keys() const;

   void test_all();
   void produce_block();
   void create_account();

   void create_account_manually(string& name);
   void get_account(string& name);

   chainbase::database* chain_db;
   block_log* chain_log;
   fork_database* chain_fdb;
   native_contract::native_contract_chain_initializer* chain_initializer;
   testing_blockchain* chain;


protected:
   std::vector<fc::temp_directory> anonymous_temp_dirs;
   std::map<std::string, fc::temp_directory> named_temp_dirs;
   std::map<public_key_type, private_key_type> key_ring;
   native_contract::genesis_state_type default_genesis_state;

};

/**
 * @brief The testing_blockchain class wraps chain_controller and eliminates some of the boilerplate for common
 * operations on the blockchain during testing.
 *
 * testing_blockchains have an optional ID, which is passed to the constructor. If two testing_blockchains are created
 * with the same ID, they will have the same data directory. If no ID, or an empty ID, is provided, the database will
 * have a unique data directory which no subsequent testing_blockchain will be assigned.
 *
 * testing_blockchain helps with producing blocks, or missing blocks, via the @ref produce_blocks and @ref miss_blocks
 * methods. To produce N blocks, simply call produce_blocks(N); to miss N blocks, call miss_blocks(N). Note that missing
 * blocks has no effect on the blockchain until the next block, following the missed blocks, is produced.
 */
class testing_blockchain : public chain_controller {
public:
   testing_blockchain(chainbase::database& db, fork_database& fork_db, block_log& blocklog,
                     chain_initializer_interface& initializer, testing_fixture& fixture);

   /**
    * @brief Publish the provided contract to the blockchain, owned by owner
    * @param owner The account to publish the contract under
    * @param contract_wast The WAST of the contract
    */
   void set_contract(AccountName owner, const char* contract_wast);

   /**
    * @brief Produce new blocks, adding them to the blockchain, optionally following a gap of missed blocks
    * @param count Number of blocks to produce
    * @param blocks_to_miss Number of block intervals to miss a production before producing the next block
    *
    * Creates and adds @ref count new blocks to the blockchain, after going @ref blocks_to_miss intervals without
    * producing a block.
    */
   void produce_blocks(uint32_t count = 1, uint32_t blocks_to_miss = 0);

   /**
    * @brief Sync this blockchain with other
    * @param other Blockchain to sync with
    *
    * To sync the blockchains, all blocks from one blockchain which are unknown to the other are pushed to the other,
    * then the same thing in reverse. Whichever blockchain has more blocks will have its blocks sent to the other
    * first.
    *
    * Blocks not on the main fork are ignored.
    */
   void sync_with(testing_blockchain& other);

   /// @brief Get the liquid balance belonging to the named account
   Asset get_liquid_balance(const types::AccountName& account);
   /// @brief Get the staked balance belonging to the named account
   Asset get_staked_balance(const types::AccountName& account);
   /// @brief Get the unstaking balance belonging to the named account
   Asset get_unstaking_balance(const types::AccountName& account);

   /// @brief Get the set of producers approved by the named account
   std::set<AccountName> get_approved_producers(const AccountName& account);
   /// @brief Get the specified block producer's signing key
   PublicKey get_block_signing_key(const AccountName& producerName);

   /// @brief Attempt to sign the provided transaction using the keys available to the testing_fixture
   void sign_transaction(SignedTransaction& trx) const;

   /// @brief Override push_transaction to apply testing policies
   /// If transactions are being held for review, transaction will be held after testing policies are applied
   fc::optional<ProcessedTransaction> push_transaction(SignedTransaction trx, uint32_t skip_flags = 0);
   /// @brief Review and optionally push last held transaction
   /// @tparam F A callable with signature `bool f(SignedTransaction&, uint32_t&)`
   /// @param reviewer Callable which inspects and potentially alters the held transaction and skip flags, and returns
   /// whether it should be pushed or not
   template<typename F>
   fc::optional<ProcessedTransaction> review_transaction(F&& reviewer) {
      if (reviewer(review_storage.first, review_storage.second))
         return chain_controller::push_transaction(review_storage.first, review_storage.second);
      return {};
   }

   /// @brief Set whether testing_blockchain::push_transaction checks signatures by default
   /// @param skip_sigs If true, push_transaction will skip signature checks; otherwise, no changes will be made
   void set_skip_transaction_signature_checking(bool skip_sigs) {
      skip_trx_sigs = skip_sigs;
   }
   /// @brief Set whether testing_blockchain::push_transaction attempts to sign transactions or not
   void set_auto_sign_transactions(bool auto_sign) {
      auto_sign_trxs = auto_sign;
   }
   /// @brief Set whether testing_blockchain::push_transaction holds transactions for review or not
   void set_hold_transactions_for_review(bool hold_trxs) {
      hold_for_review = hold_trxs;
   }

protected:
   chainbase::database& db;
   testing_fixture& fixture;
   std::pair<SignedTransaction, uint32_t> review_storage;
   bool skip_trx_sigs = true;
   bool auto_sign_trxs = false;
   bool hold_for_review = false;
};

} }

namespace python {
namespace test {
void test_create_account_(string& name);
void test_get_account_(string& name);

void test1_();
void test2_();

void test_all_();
void test_db_();


void test_start_();
void test_create_account_();
void produce_block_();
void test_end_();

void create_account_manually_(string& name);
void get_account_(string& name);
void get_block_num_();
void get_block_log_num_();

}
}

#endif /* PROGRAMS_PYEOS_INTERFACE_EOSTEST__HPP_ */
