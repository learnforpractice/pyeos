#include "eosapi_.hpp"

#include <fc/time.hpp>
#include <eosio/chain/block_summary_object.hpp>
#include <eosio/wallet_plugin/wallet_plugin.hpp>
#include <eosio/chain/wast_to_wasm.hpp>
#include <eosio/chain/contracts/types.hpp>

#include "micropython/mpeoslib.h"

#include "fc/bitutil.hpp"
#include "json.hpp"
#include "pyobject.hpp"
#include "wallet_.h"

#include "localize.hpp"
#include <regex>

using namespace eosio::client::localize;
using namespace eosio::chain;

auto tx_expiration = fc::seconds(30);
bool tx_force_unique = false;

uint32_t tx_cf_cpu_usage = 0;
uint32_t tx_net_usage = 0;

uint32_t tx_max_cpu_usage = 0;
uint32_t tx_max_net_usage = 0;

uint64_t string_to_uint64_(string str) {
   try {
      return name(str).value;
   } catch (...) {
   }
   return 0;
}

string uint64_to_string_(uint64_t n) {
   return name(n).to_string();
}

static uint64_t get_microseconds() {
   struct timeval  tv;
   gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000000LL + tv.tv_usec * 1LL ;
}

string convert_to_eth_address(string& name) {
   uint64_t n = string_to_uint64_(name);
   char address[20];
   memset(address, 0, sizeof(address));
   ((uint64_t*)address)[0] = n;
   return "0x" + fc::to_hex(address, sizeof(address));
}

string convert_from_eth_address(string& eth_address) {
   char address[20];
   memset(address, 0, sizeof(address));
   if (eth_address[0] == '0' && eth_address[1] == 'x') {
      eth_address = string(eth_address.begin()+2, eth_address.end());
   }
   fc::from_hex(eth_address, address, sizeof(address));
   return uint64_to_string_(*((uint64_t*)address));
}


uint32_t now2_() { return fc::time_point::now().sec_since_epoch(); }

chain_controller& get_db() { return app().get_plugin<chain_plugin>().chain(); }

wallet_manager& get_wm() {
   return app().get_plugin<wallet_plugin>().get_wallet_manager();
}

inline std::vector<name> sort_names(std::vector<name>&& names) {
   std::sort(names.begin(), names.end());
   auto itr = std::unique(names.begin(), names.end());
   names.erase(itr, names.end());
   return names;
}

fc::variant json_from_file_or_string(const string& file_or_str, fc::json::parse_type ptype = fc::json::legacy_parser)
{
   return fc::json::from_string(file_or_str, ptype);
}

authority parse_json_authority(const std::string& authorityJsonOrFile) {
   try {
      return json_from_file_or_string(authorityJsonOrFile).as<authority>();
   } EOS_RETHROW_EXCEPTIONS(authority_type_exception, "Fail to parse Authority JSON '${data}'", ("data",authorityJsonOrFile))
}

authority parse_json_authority_or_key(const std::string& authorityJsonOrFile) {
   if (boost::istarts_with(authorityJsonOrFile, "EOS")) {
      try {
         return authority(public_key_type(authorityJsonOrFile));
      } EOS_RETHROW_EXCEPTIONS(public_key_type_exception, "Invalid public key: ${public_key}", ("public_key", authorityJsonOrFile))
   } else {
      return parse_json_authority(authorityJsonOrFile);
   }
}


vector<uint8_t> assemble_wast(const std::string& wast) {
   IR::Module module;
   std::vector<WAST::Error> parseErrors;
   WAST::parseModule(wast.c_str(), wast.size(), module, parseErrors);
   if (parseErrors.size()) {
      // Print any parse errors;
      std::cerr << "Error parsing WebAssembly text file:" << std::endl;
      for (auto& error : parseErrors) {
         std::cerr << ":" << error.locus.describe() << ": "
                   << error.message.c_str() << std::endl;
         std::cerr << error.locus.sourceLine << std::endl;
         std::cerr << std::setw(error.locus.column(8)) << "^" << std::endl;
      }
      FC_ASSERT(!"error parsing wast");
   }

   try {
      // Serialize the WebAssembly module.
      Serialization::ArrayOutputStream stream;
      WASM::serialize(stream, module);
      return stream.getBytes();
   } catch (Serialization::FatalSerializationException& exception) {
      std::cerr << "Error serializing WebAssembly binary file:" << std::endl;
      std::cerr << exception.message << std::endl;
      throw;
   }
}

read_only::get_info_results get_info() {
   auto& db = get_db();
   auto itoh = [](uint32_t n, size_t hlen = sizeof(uint32_t)<<1) {
    static const char* digits = "0123456789abcdef";
    std::string r(hlen, '0');
    for(size_t i = 0, j = (hlen - 1) * 4 ; i < hlen; ++i, j -= 4)
      r[i] = digits[(n>>j) & 0x0f];
    return r;
   };

   return {
       itoh(static_cast<uint32_t>(app().version())),
       db.head_block_num(),
       db.last_irreversible_block_num(),
       db.head_block_id(),
       db.head_block_time(),
       db.head_block_producer()};
}

string generate_nonce_value() {
   return fc::to_string(fc::time_point::now().time_since_epoch().count());
}

chain::action generate_nonce() {
   auto v = generate_nonce_value();
   variant nonce = fc::mutable_variant_object()
         ("value", v);
   return chain::action( {}, config::system_account_name, "nonce", fc::raw::pack(nonce));
}

fc::variant determine_required_keys(const signed_transaction& trx) {
   // TODO better error checking
//   const auto& public_keys = call(wallet_host, wallet_port, wallet_public_keys);
   auto& wallet_mgr = app().get_plugin<wallet_plugin>().get_wallet_manager();
//   flat_set<public_key_type> wallet_manager::get_public_keys() {
   const auto& public_keys = wallet_mgr.get_public_keys();

   eosio::chain_apis::read_only::get_required_keys_params get_arg = {fc::variant((transaction)trx), public_keys};
//   auto get_arg = fc::mutable_variant_object
//           ("transaction", (transaction)trx)
//           ("available_keys", variant(public_keys));
//   read_only::get_required_keys_result
//   const auto& required_keys = call(host, port, get_required_keys, get_arg);
   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
   auto results = ro_api.get_required_keys(get_arg);
   return fc::variant(results.required_keys);
}

PyObject* push_transaction(signed_transaction& trx, bool sign, int32_t extra_kcpu = 1000, packed_transaction::compression_type compression = packed_transaction::none, bool async=false) {
   auto info = get_info();
   trx.expiration = info.head_block_time + tx_expiration;
   trx.set_reference_block(info.head_block_id);

   if (tx_force_unique) {
      trx.context_free_actions.emplace_back( generate_nonce() );
   }

   auto required_keys = determine_required_keys(trx);
   size_t num_keys = required_keys.is_array() ? required_keys.get_array().size() : 1;

   trx.max_kcpu_usage = (tx_max_cpu_usage + 1023)/1024;
   trx.max_net_usage_words = (tx_max_net_usage + 7)/8;

   if (sign) {
      sign_transaction(trx);
   }

   auto rw = app().get_plugin<chain_plugin>().get_read_write_api();
   chain_apis::read_write::push_transaction_results result;

   bool success = false;
   uint64_t cost_time = 0;
   try {
      if (async) {
         app().get_plugin<chain_plugin>().chain().push_transaction_async(packed_transaction(trx, compression), skip_nothing);
      } else {
         auto params = fc::variant(packed_transaction(trx, compression)).get_object();
         cost_time = get_microseconds();
         result = rw.push_transaction(params);
         cost_time = get_microseconds() - cost_time;
         success = true;
      }
   } catch (fc::assert_exception& e) {
      elog(e.to_detail_string());
   } catch (fc::exception& e) {
      elog(e.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }

   if (success) {
      PyObject* dict = python::json::to_string(result);
      string _key = string("cost_time");
      PyObject* key = py_new_string(_key);
      PyObject* value = py_new_uint64(cost_time);
      dict_add(dict, key, value);
      return dict;
   }
   return py_new_none();
}

PyObject* push_actions(std::vector<chain::action>&& actions, bool sign, packed_transaction::compression_type compression = packed_transaction::none ) {
   signed_transaction trx;
   trx.actions = std::forward<decltype(actions)>(actions);

//   wlog("++++++++++++++++++++++push_transaction");
   return push_transaction(trx, sign, 10000000, compression);
}

PyObject* send_actions(std::vector<chain::action>&& actions, bool sign, packed_transaction::compression_type compression = packed_transaction::none) {
   return push_actions(std::forward<decltype(actions)>(actions), sign, compression);
}

bool gen_transaction(signed_transaction& trx, bool sign, int32_t extra_kcpu = 1000, packed_transaction::compression_type compression = packed_transaction::none) {
   auto info = get_info();
   trx.expiration = info.head_block_time + tx_expiration;
   trx.set_reference_block(info.head_block_id);

   if (tx_force_unique) {
      trx.context_free_actions.emplace_back( generate_nonce() );
   }

   auto required_keys = determine_required_keys(trx);
   size_t num_keys = required_keys.is_array() ? required_keys.get_array().size() : 1;

   trx.max_kcpu_usage = (tx_max_cpu_usage + 1023)/1024;
   trx.max_net_usage_words = (tx_max_net_usage + 7)/8;

   if (sign) {
      sign_transaction(trx);
   }
   return true;
}

PyObject* push_transactions_(vector<string>& contracts, vector<string>& functions, vector<string>& args,
      vector<map<string, string>>& permissions, bool sign, bool rawargs) {
   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();

   vector<vector<chain::permission_level>> accountPermissions;

   for (int i=0;i<permissions.size();i++) {
      auto& per = permissions[i];
      vector<chain::permission_level> _v;
      for (auto it = per.begin(); it != per.end(); it++) {
         _v.push_back(chain::permission_level{name(it->first), name(it->second)});
      }
      accountPermissions.push_back(_v);
   }

   vector<chain::action> actions;

   eosio::chain_apis::read_only::abi_json_to_bin_params params;
   for (int i=0;i<functions.size();i++) {
         string& action = functions[i];
         string& arg = args[i];
         if (!rawargs) {
         params = {contracts[i], action, fc::json::from_string(arg)};
      } else {
         std::vector<char> v(arg.begin(), arg.end());
         params = {contracts[i], action, fc::variant(v)};
      }
      auto result = ro_api.abi_json_to_bin(params);
      actions.emplace_back(accountPermissions[i], contracts[i], action, result.binargs);
   }

   packed_transaction::compression_type compression = packed_transaction::none;
   vector<signed_transaction* > trxs;

   for (auto& action: actions) {
      signed_transaction *trx = new signed_transaction();
      trxs.push_back(trx);
      trx->actions.push_back(action);
      gen_transaction(*trx, sign, 10000000, compression);
   }

   uint64_t cost_time = get_microseconds();

   auto rw = app().get_plugin<chain_plugin>().get_read_write_api();
   for (auto& strx : trxs) {
      chain_apis::read_write::push_transaction_results result;
      bool success = false;
      try {
         auto params = fc::variant(packed_transaction(*strx, compression)).get_object();
         result = rw.push_transaction(params);
         success = true;
      } catch (fc::assert_exception& e) {
         elog(e.to_detail_string());
      } catch (fc::exception& e) {
         elog(e.to_detail_string());
      } catch (boost::exception& ex) {
         elog(boost::diagnostic_information(ex));
      }
   }
   cost_time = get_microseconds() - cost_time;

   for (auto& st : trxs) {
      free(st);
   }

   return py_new_uint64(cost_time);
}


PyObject* push_transactions2_(vector<vector<chain::action>>& vv, bool sign, uint64_t skip_flag, bool async) {
   packed_transaction::compression_type compression = packed_transaction::none;
   vector<signed_transaction* > trxs;

   uint64_t cost_time = 0;

   try {
      for (auto& v: vv) {
         signed_transaction *trx = new signed_transaction();
         trxs.push_back(trx);
         for(auto& action: v) {
            trx->actions.push_back(std::move(action));
         }
         gen_transaction(*trx, sign, 10000000, compression);
      }
      cost_time = get_microseconds();
      auto rw = app().get_plugin<chain_plugin>().get_read_write_api();

      for (auto& strx : trxs) {
         if (async) {
            app().get_plugin<chain_plugin>().chain().push_transaction_async(packed_transaction(std::move(*strx), compression), skip_flag);
         } else {
            chain_apis::read_write::push_transaction_results result;
//            auto params = fc::variant(packed_transaction(std::move(*strx), compression)).get_object();
//            result = rw.push_transaction(params);
            auto ptrx = packed_transaction(std::move(*strx), compression);
            app().get_plugin<chain_plugin>().chain().push_transaction(ptrx, skip_flag);
         }
      }
   } catch (fc::assert_exception& e) {
      elog(e.to_detail_string());
   } catch (fc::exception& e) {
      elog(e.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }

   cost_time = get_microseconds() - cost_time;


   for (auto& st : trxs) {
      delete st;
   }

   return py_new_uint64(cost_time);
}


PyObject* gen_transaction_(vector<chain::action>& v, int expiration) {
   packed_transaction::compression_type compression = packed_transaction::none;
   vector<signed_transaction* > trxs;

   uint64_t cost_time = 0;

   try {
      signed_transaction trx;
      for(auto& action: v) {
         trx.actions.push_back(std::move(action));
      }

      auto info = get_info();
      trx.expiration = info.head_block_time + fc::seconds(expiration);
      trx.set_reference_block(info.head_block_id);

      trx.max_kcpu_usage = (tx_max_cpu_usage + 1023)/1024;
      trx.max_net_usage_words = (tx_max_net_usage + 7)/8;
      return python::json::to_string(fc::variant(trx));

   } catch (fc::assert_exception& e) {
      elog(e.to_detail_string());
   } catch (fc::exception& e) {
      elog(e.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   return py_new_none();
}

PyObject* sign_transaction_(string& trx_json_to_sign, string& str_private_key) {
   try {
      signed_transaction trx = python::json::from_string(trx_json_to_sign).as<signed_transaction>();

      auto priv_key = fc::crypto::private_key::regenerate(*utilities::wif_to_key(str_private_key));
      trx.sign(priv_key, chain_id_type{});
      return python::json::to_string(fc::variant(trx));

   } catch (fc::assert_exception& e) {
      elog(e.to_detail_string());
   } catch (fc::exception& e) {
      elog(e.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   return py_new_none();

}

PyObject* push_raw_transaction_(string& signed_trx) {
   bool success = false;

   auto rw = app().get_plugin<chain_plugin>().get_read_write_api();
   chain_apis::read_write::push_transaction_results result;

   try {
      signed_transaction trx = python::json::from_string(signed_trx).as<signed_transaction>();
      packed_transaction::compression_type compression = packed_transaction::none;

      auto params = fc::variant(packed_transaction(trx, compression)).get_object();
      result = rw.push_transaction(params);
      success = true;
   } catch (fc::assert_exception& e) {
      elog(e.to_detail_string());
   } catch (fc::exception& e) {
      elog(e.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }

   if (success) {
      return python::json::to_string(result);
   }
   return py_new_none();
}


PyObject* push_transaction2_(void* signed_trx, bool sign) {

   if (signed_trx == NULL) {
      return py_new_none();
   }

   signed_transaction& trx = *((signed_transaction *)signed_trx);

   auto info = get_info();
   trx.expiration = info.head_block_time + 100;
   trx.ref_block_num = fc::endian_reverse_u32(info.head_block_id._hash[0]);
   trx.ref_block_prefix = info.head_block_id._hash[1];
//   boost::sort(trx.scope);

   if (sign) {
      sign_transaction(trx);
   }

   auto rw = app().get_plugin<chain_plugin>().get_read_write_api();
   chain_apis::read_write::push_transaction_results result;

   bool success = false;
//   PyThreadState* state = PyEval_SaveThread();
   try {
      result = rw.push_transaction(fc::variant(trx).get_object());
      success = true;
   } catch (fc::assert_exception& e) {
      elog(e.to_detail_string());
   } catch (fc::exception& e) {
      elog(e.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }

//   PyEval_RestoreThread(state);

   if (success) {
      return python::json::to_string(result);
   }
   return py_new_none();
}


int produce_block_() {
   return app().get_plugin<producer_plugin>().produce_block();
}

PyObject* create_key_() {
   auto pk    = private_key_type::generate();
   auto privs = string(pk);
   auto pubs  = string(pk.get_public_key());

   PyDict dict;
   string key;

   key = "public";
   dict.add(key, pubs);

   key = "private";
   dict.add(key, privs);
   return dict.get();
}

PyObject* get_public_key_(string& wif_key) {
   auto priv_key = eosio::utilities::wif_to_key(wif_key);
   if (!priv_key) {
      return py_new_none();
   }
   string pub_key = priv_key->get_public_key().to_base58();
   return py_new_string(pub_key);
}


PyObject* create_account_(string creator, string newaccount, string owner,
                          string active, int sign) {
   try {
      auto owner_auth = eosio::chain::authority{1, {{public_key_type(owner), 1}}, {}};
      auto active_auth = eosio::chain::authority{1, {{public_key_type(active), 1}}, {}};
      auto recovery_auth = eosio::chain::authority{1, {}, {{{creator, "active"}, 1}}};

      vector<chain::action> actions;
      actions.emplace_back( vector<chain::permission_level>{{creator,"active"}},
                                contracts::newaccount{creator, newaccount, owner_auth, active_auth, recovery_auth});

      return send_actions(std::move(actions), sign);

   } catch (fc::assert_exception& e) {
      elog(e.to_detail_string());
   } catch (fc::exception& e) {
      elog(e.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   return py_new_none();
}

PyObject* get_info_() {
   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
   chain_apis::read_only::get_info_params params = {};
   chain_apis::read_only::get_info_results results = ro_api.get_info(params);
   return python::json::to_string(results);

   PyDict dict;
   const chain_controller& db = get_db();
   string key;
   string value;

   key = "head_block_num";
   dict.add(key, db.head_block_num());

   key = "last_irreversible_block_num";
   dict.add(key, db.last_irreversible_block_num());

   key = "head_block_id";
   value = db.head_block_id().str();
   dict.add(key, value);

   key = "head_block_time";
   value = string(db.head_block_time());
   dict.add(key, value);

   key = "head_block_producer";
   value = db.head_block_producer().to_string();
   dict.add(key, value);

   return dict.get();
}
/*
 {
 "previous": "0000000000000000000000000000000000000000000000000000000000000000",
 "timestamp": "2017-09-02T13:38:39",
 "transaction_merkle_root":
 "0000000000000000000000000000000000000000000000000000000000000000", "producer":
 "initr", "producer_changes": [], "producer_signature":
 "1f6a8a50b6993bfa07d0b195c4e5378d4f105e0c6cfe6beed5050948531b7000ef2d75f463d7a095d5b931c3923a78e04406308d2cbd2a5435422046363f948940",
 "cycles": [],
 "id": "00000001b107f27f37bb944c10952a758dc44129fe0310b7a1b76d47a67ad908",
 "block_num": 1,
 "ref_block_prefix": 1284815671
 }
 */

PyObject* get_block_(char* num_or_id) {
   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
   try {
      chain_apis::read_only::get_block_params params = {string(num_or_id)};
      auto results = ro_api.get_block(params);
      return python::json::to_string(results);
   } catch (fc::bad_cast_exception& ex) { /* do nothing */
   } catch (const fc::exception& e) {
      elog((e.to_detail_string()));
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   return py_new_none();
}

/*
 struct get_account_results {
 Name                       name;
 uint64_t                   eos_balance       = 0;
 uint64_t                   staked_balance    = 0;
 uint64_t                   unstaking_balance = 0;
 fc::time_point_sec         last_unstaking_time;
 optional<producer_info>    producer;
 optional<types::abi>       abi;
 };
 */
PyObject* get_account_(const char* _name) {
//   using namespace native::eosio;
   PyArray arr;
   PyDict dict;

   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();

   eosio::chain_apis::read_only::get_account_results result;
   eosio::chain_apis::read_only::get_account_params params = {chain::name(_name).value};
   result = ro_api.get_account(params);
   return python::json::to_string(result);
}

PyObject* get_accounts_(char* public_key) {
   PyArray arr;
   try {
      if (public_key == NULL) {
         return arr.get();
      }
      auto ro_api = app().get_plugin<account_history_plugin>().get_read_only_api();
      auto rw_api = app().get_plugin<account_history_plugin>().get_read_write_api();

      eosio::account_history_apis::read_only::get_key_accounts_params params =
         {
             chain::public_key_type{}
         };

      eosio::account_history_apis::read_only::get_key_accounts_results results =
          ro_api.get_key_accounts(params);

      for (auto it = results.account_names.begin();
           it != results.account_names.end(); ++it) {
         arr.append(string(*it));
      }
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   }
   return arr.get();
}

PyObject* get_currency_balance_(string& _code, string& _account, string& _symbol) {
//   using namespace native::eosio;
   PyArray arr;
   PyDict dict;

   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();

   eosio::chain_apis::read_only::get_currency_balance_params params = {chain::name(_code), chain::name(_account), _symbol};
   auto result = ro_api.get_currency_balance(params);
   return python::json::to_string(result);
}


PyObject* get_controlled_accounts_(const char* account_name) {
   PyArray arr;
   try {
      if (account_name == NULL) {
         return arr.get();
      }
      auto ro_api = app().get_plugin<account_history_plugin>().get_read_only_api();
      eosio::account_history_apis::read_only::get_controlled_accounts_params params = {
            chain::name(account_name).value
      };
      eosio::account_history_apis::read_only::get_controlled_accounts_results
          results = ro_api.get_controlled_accounts(params);
      for (auto it = results.controlled_accounts.begin();
           it != results.controlled_accounts.end(); it++) {
         arr.append(string(*it));
      }
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   }
   return arr.get();
}

int get_transaction_(string& id, string& result) {
   eosio::account_history_apis::read_only::get_transaction_params params = {
       chain::transaction_id_type(id)};
   try {
      auto ro_api =
          app().get_plugin<account_history_plugin>().get_read_only_api();
      eosio::account_history_apis::read_only::get_transaction_results results =
          ro_api.get_transaction(params);
      result = fc::json::to_string(fc::variant(results));
      return 0;
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   }
   return -1;
}

int get_transactions_(string& account_name, int skip_seq, int num_seq,
                      string& result) {
   try {
      const eosio::account_history_apis::read_only::get_transactions_params
          params = {chain::account_name(account_name), skip_seq, num_seq};

      auto ro_api = app().get_plugin<account_history_plugin>().get_read_only_api();

      eosio::account_history_apis::read_only::get_transactions_results results =
          ro_api.get_transactions(params);

      result = fc::json::to_string(results);
      return 0;
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   }
   return -1;
}


PyObject* transfer_(string& sender, string& recipient, int amount, string memo, bool sign) {

   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();

   auto transfer = fc::mutable_variant_object
         ("from", sender)
         ("to", recipient)
         ("quantity", asset(amount))
         ("memo", memo);

   eosio::chain_apis::read_only::abi_json_to_bin_params params = {
         name(config::system_account_name), "transfer", transfer
   };

   auto result = ro_api.abi_json_to_bin(params);

   std::vector<chain::action> actions;
   actions.emplace_back(vector<chain::permission_level>{{sender,"active"}},
                            config::system_account_name,
                            "transfer",
                            result.binargs);

   if (tx_force_unique) {
      actions.emplace_back( generate_nonce() );
   }

   return send_actions(std::move(actions), sign);

}

PyObject* push_message_(string& contract, string& action, string& args, map<string, string>& permissions,
                        bool sign, bool rawargs) {
//   wlog("+++++++++++++++++push_message:${n}", ("n", contract));
   try {
      //      ilog("Converting argument to binary...");
      auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();

      vector<chain::permission_level> accountPermissions;
      for (auto it = permissions.begin(); it != permissions.end(); it++) {
         accountPermissions.push_back(chain::permission_level{name(it->first), name(it->second)});
      }

      vector<chain::action> actions;

      if (action.size() == 0) {//evm
            string _args;
            if (args[0] == '0' && args[1] == 'x') {
               _args = string(args.begin()+2, args.end());
            } else {
               _args = args;
            }
            bytes v;
            v.resize(0);
            v.resize(_args.size()/2);
            fc::from_hex(_args, v.data(), v.size());
         actions.emplace_back(accountPermissions, contract, action, v);
      } else {
         std::vector<char> v;
         if (rawargs) {
            v = std::vector<char>(args.begin(), args.end());
         } else {
            eosio::chain_apis::read_only::abi_json_to_bin_params params;
            params = {contract, action, fc::json::from_string(args)};
            auto result = ro_api.abi_json_to_bin(params);
            v = result.binargs;
         }
         actions.emplace_back(accountPermissions, contract, action, v);
      }

      if (tx_force_unique) {
         actions.emplace_back( generate_nonce() );
      }

      return send_actions(std::move(actions), sign);

   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   /*
    auto obj =
    get_db().get_database().get<eosio::chain::block_summary_object>((uint16_t)trx.refBlockNum);
    ilog("obj.block_id._hash[0] ${n} ", ("n",
    fc::endian_reverse_u32(obj.block_id._hash[0]))); ilog("trx.refBlockNum ${n}
    ", ("n", trx.refBlockNum) );
    */
   return py_new_none();
}

PyObject* push_messages_(vector<string>& contracts, vector<string>& functions, vector<string>& args,
      vector<map<string, string>>& permissions, bool sign, bool rawargs) {
   signed_transaction trx;
//   ilog("+++++++++++++++++push_message:${n}", ("n", contract));
   try {
      //      ilog("Converting argument to binary...");
      auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
      auto rw_api = app().get_plugin<chain_plugin>().get_read_write_api();

      vector<vector<chain::permission_level>> accountPermissions;

      for (int i=0;i<permissions.size();i++) {
         auto& per = permissions[i];
         vector<chain::permission_level> _v;
         for (auto it = per.begin(); it != per.end(); it++) {
            _v.push_back(chain::permission_level{name(it->first), name(it->second)});
         }
         accountPermissions.push_back(_v);
      }

      vector<chain::action> actions;

      if (functions.size() == 0) {//evm
//            for (auto& arg : args) {
         for (int i=0;i<args.size();i++) {
            auto& arg = args[i];
            string _args;
            if (arg[0] == '0' && arg[1] == 'x') {
               _args = string(arg.begin()+2, arg.end());
            } else {
               _args = arg;
            }
            bytes v;
            v.resize(0);
            v.resize(_args.size()/2);
            fc::from_hex(_args, v.data(), v.size());
            actions.emplace_back(accountPermissions[i], contracts[i], "", v);
         }
      } else {
         eosio::chain_apis::read_only::abi_json_to_bin_params params;
         for (int i=0;i<functions.size();i++) {
               string& action = functions[i];
               string& arg = args[i];
               if (!rawargs) {
               params = {contracts[i], action, fc::json::from_string(arg)};
            } else {
               std::vector<char> v(arg.begin(), arg.end());
               params = {contracts[i], action, fc::variant(v)};
            }
            auto result = ro_api.abi_json_to_bin(params);
            actions.emplace_back(accountPermissions[i], contracts[i], action, result.binargs);
         }
      }

      if (tx_force_unique) {
         actions.emplace_back( generate_nonce() );
      }
      PyObject* ret = send_actions(std::move(actions), sign);
      return ret;
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }

   return py_new_none();
}

PyObject* push_messages_ex_(string& contract, vector<string>& functions, vector<string>& args,
                              map<string, string>& permissions, bool sign, bool rawargs) {
   signed_transaction trx;
//   ilog("+++++++++++++++++push_message:${n}", ("n", contract));
   try {
      //      ilog("Converting argument to binary...");
      auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
      auto rw_api = app().get_plugin<chain_plugin>().get_read_write_api();

      vector<chain::permission_level> accountPermissions;
      for (auto it = permissions.begin(); it != permissions.end(); it++) {
         accountPermissions.push_back(chain::permission_level{name(it->first), name(it->second)});
      }

      vector<chain::action> actions;

      if (functions.size() == 0) {//evm
//            for (auto& arg : args) {
         for (int i=0;i<args.size();i++) {
            auto& arg = args[i];
            string _args;
            if (arg[0] == '0' && arg[1] == 'x') {
               _args = string(arg.begin()+2, arg.end());
            } else {
               _args = arg;
            }
            bytes v;
            v.resize(0);
            v.resize(_args.size()/2);
            fc::from_hex(_args, v.data(), v.size());
            actions.emplace_back(accountPermissions, contract, "", v);
         }
      } else {
         eosio::chain_apis::read_only::abi_json_to_bin_params params;
         for (int i=0;i<functions.size();i++) {
               string action = functions[i];
               string arg = args[i];
               if (!rawargs) {
               params = {contract, action, fc::json::from_string(arg)};
            } else {
               std::vector<char> v(arg.begin(), arg.end());
               params = {contract, action, fc::variant(v)};
            }
            auto result = ro_api.abi_json_to_bin(params);
            actions.emplace_back(accountPermissions, contract, action, result.binargs);
         }
      }

      if (tx_force_unique) {
         actions.emplace_back( generate_nonce() );
      }
      PyObject* ret = send_actions(std::move(actions), sign);
      return ret;
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }

   return py_new_none();
}

PyObject* push_evm_message_(string& contract, string& args, map<string, string>& permissions,
                        bool sign, bool rawargs) {
   signed_transaction trx;

   try {
      //      ilog("Converting argument to binary...");
      auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
      auto rw_api = app().get_plugin<chain_plugin>().get_read_write_api();
      eosio::chain_apis::read_only::abi_json_to_bin_params params;
      if (!rawargs) {
         params = {contract, "", fc::json::from_string(args)};
      } else {
         std::vector<char> v(args.begin(), args.end());
         params = {contract, "", fc::variant(v)};
      }

      vector<chain::permission_level> accountPermissions;
      for (auto it = permissions.begin(); it != permissions.end(); it++) {
         accountPermissions.push_back(chain::permission_level{name(it->first), name(it->second)});
      }

      vector<chain::action> actions;

      string _args;
      if (args[0] == '0' && args[1] == 'x') {
         _args = string(args.begin()+2, args.end());
      } else {
         _args = args;
      }
      bytes v;
      v.resize(0);
      v.resize(_args.size()/2);
      fc::from_hex(_args, v.data(), v.size());
      actions.emplace_back(accountPermissions, contract, "", v);

      if (tx_force_unique) {
         actions.emplace_back( generate_nonce() );
      }

      return send_actions(std::move(actions), sign);

   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   return py_new_none();
}


PyObject* set_contract_(string& account, string& srcPath, string& abiPath,
                        int vm_type, bool sign) {
   try {
      std::string _src;
      contracts::setcode handler;
      handler.vmtype = vm_type;
      if (vm_type == 0) {
         std::cout << localized("Reading WAST...") << std::endl;
         fc::read_file_contents(srcPath, _src);

         vector<uint8_t> wasm;
         const string binary_wasm_header = "\x00\x61\x73\x6d";
         if(_src.compare(0, 4, binary_wasm_header) == 0) {
            std::cout << localized("Using already assembled WASM...") << std::endl;
            wasm = vector<uint8_t>(_src.begin(), _src.end());
         }
         else {
            std::cout << localized("Assembling WASM...") << std::endl;
            if (false) {
               wasm = assemble_wast(_src);
            } else {
               wasm = wast_to_wasm(_src);
            }
         }
         handler.account = account;
         handler.code.assign(wasm.begin(), wasm.end());
      } else if (vm_type == 1) {
         fc::read_file_contents(srcPath, _src);
         handler.account = account;
         handler.code.resize(0);
         handler.code.resize(_src.length()+1);
         if (srcPath[srcPath.length() - 3] == 'm') {//compiled code
               handler.code.data()[0] = '\x01';
         } else {
               handler.code.data()[0] = '\x00';
         }
         memcpy(&handler.code.data()[1], _src.c_str(), _src.length());
//         handler.code.assign(_src.begin(), _src.end());
      } else if (vm_type == 2) {
         fc::read_file_contents(srcPath, _src);
         _src = fc::trim(_src);
         bytes bin;
         bin.resize(0);
         bin.resize(_src.size()/2);
         fc::from_hex(_src, bin.data(), bin.size());
         handler.account = account;
         handler.code.assign(bin.begin(), bin.end());
      }  else if (vm_type == 3) {
         fc::read_file_contents(srcPath, _src);
         handler.account = account;
         handler.code.resize(0);
         handler.code.resize(_src.length());
         memcpy(handler.code.data(), _src.c_str(), _src.length());
//         handler.code.assign(_src.begin(), _src.end());
      }

      vector<chain::action> actions;
      actions.emplace_back( vector<chain::permission_level>{{account,"active"}}, handler);

      if (!abiPath.empty()) {
         contracts::setabi handler;
         handler.account = account;
         try {
            handler.abi = fc::json::from_file(abiPath).as<contracts::abi_def>();
         } EOS_CAPTURE_AND_RETHROW(abi_type_exception)
         actions.emplace_back( vector<chain::permission_level>{{account,"active"}}, handler);
      }

      std::cout << localized("Publishing contract...") << std::endl;

      uint64_t cost_time = get_microseconds();
      auto ret = send_actions(std::move(actions), sign, packed_transaction::zlib);
      cost_time = get_microseconds() - cost_time;
      wlog("cost time: ${n}", ("n", cost_time));
      return ret;
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   return py_new_none();
}

PyObject* set_evm_contract_(string& eth_address, string& sol_bin, bool sign) {

   try {
      string account = convert_from_eth_address(eth_address);

      contracts::setcode handler;
      handler.vmtype = 2;

      sol_bin = fc::trim(sol_bin);
      bytes bin;
      bin.resize(0);
      bin.resize(sol_bin.size()/2);
      fc::from_hex(sol_bin, bin.data(), bin.size());
      handler.account = account;
      handler.code.assign(bin.begin(), bin.end());

      vector<chain::action> actions;
      actions.emplace_back( vector<chain::permission_level>{{account,"active"}}, handler);

      std::cout << localized("Publishing contract...") << std::endl;
      return send_actions(std::move(actions), sign, packed_transaction::zlib);

   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   return py_new_none();
}

#include <eosio/chain/contracts/types.hpp>
using namespace eosio;

int get_code_(string& name, string& wast, string& str_abi, string& code_hash, int& vm_type) {
   try {
      chain_controller& db = app().get_plugin<chain_plugin>().chain();

      chain_apis::read_only::get_code_results result;
      result.account_name = name;
      const auto& d = db.get_database();
      const auto& accnt = d.get<account_object, by_name>(name);

      if (accnt.code.size()) {
         if (accnt.vm_type == 0) {
            result.wast = chain::wasm_to_wast((const uint8_t*)accnt.code.data(),
                                              accnt.code.size());
         } else {
            result.wast = string((const char*)accnt.code.data(), accnt.code.size());
         }
         result.code_hash = fc::sha256::hash(accnt.code.data(), accnt.code.size());
      }

      chain::contracts::abi_def abi;
      if( chain::contracts::abi_serializer::to_abi(accnt.abi, abi) ) {
         result.abi = std::move(abi);
      }

      vm_type = accnt.vm_type;
      wast = result.wast;
      code_hash = result.code_hash.str();
      str_abi = fc::json::to_string(fc::variant(result.abi));
//      abi = fc::json::to_string(get_abi_serializer().binary_to_variant("abi_def", accnt.abi));
//      abi = fc::json::to_string(result.abi);
      return 0;
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   return -1;
}

int get_table_(string& scope, string& code, string& table, string& result) {
   try {
      auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
      chain_apis::read_only::get_table_rows_params params;
      params.json = true;
      params.scope = scope;
      params.code = code;
      params.table = table;
      chain_apis::read_only::get_table_rows_result results =
          ro_api.get_table_rows(params);
      result = fc::json::to_string(results);
      return 0;
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   return -1;
}

int compile_and_save_to_buffer_(const char* src_name, const char *src_buffer, size_t src_size, char* buffer, size_t size) {
   return get_mpapi().compile_and_save_to_buffer(src_name, src_buffer, src_size, buffer, size);
}

void wast2wasm_(string& wast, string& result) {
   try {
      auto wasm = wast_to_wasm(wast);
      result = string((char *)wasm.data(), wasm.size());
   } catch (...) {
      elog("wast_to_wasm failed");
   }
}

bool is_replay_() {
   return app().get_plugin<chain_plugin>().is_replay();
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

void fc_pack_setcode_(chain::contracts::setcode _setcode, vector<char>& out) {
   out = fc::raw::pack(_setcode);
}

#include <fc/io/raw.hpp>

void fc_pack_setabi_(string& abiPath, uint64_t account, string& out) {
   contracts::setabi handler;
   handler.account = account;
   handler.abi = fc::json::from_file(abiPath).as<contracts::abi_def>();
   auto _out = fc::raw::pack(handler);
   out = string(_out.begin(), _out.end());
}

void fc_pack_uint64_(uint64_t n, string& out) {
   vector<char> _out = fc::raw::pack(n);
   out = string(_out.begin(), _out.end());
}

void fc_pack_uint8_(uint8_t n, string& out) {
   vector<char> _out = fc::raw::pack(n);
   out = string(_out.begin(), _out.end());
}

void fc_pack_updateauth(string& _account, string& _permission, string& _parent, string& _auth, uint32_t _delay, string& result) {
   authority auth = parse_json_authority_or_key(_auth);
   eosio::chain::contracts::updateauth _updateauth = {_account, _permission, _parent, auth, _delay};
   vector<char> v = fc::raw::pack(_updateauth);
   result = string(v.data(), v.size());
}

/*
account_name                      account;
permission_name                   permission;
permission_name                   parent;
authority                         data;
uint32_t                          delay;
*/


#include <frameobject.h>

PyObject* traceback_()
{
    PyThreadState *tstate;
    PyFrameObject *pyframe;
    PyArray ret_arr;

#ifdef WITH_THREAD
    tstate = PyGILState_GetThisThreadState();
#else
    tstate = PyThreadState_Get();
#endif

    if (tstate == NULL) {
        return py_new_none();
    }

    for (pyframe = tstate->frame; pyframe != NULL; pyframe = pyframe->f_back) {
       PyArray arr;
       arr.append(pyframe->f_code->co_name);
       arr.append(pyframe->f_lineno);
       ret_arr.append(arr.get());
    }
    return ret_arr.get();
}



