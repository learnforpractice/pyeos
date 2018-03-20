#include "eosapi_.hpp"

#include <fc/time.hpp>
#include "eosio/chain/block_summary_object.hpp"
#include "fc/bitutil.hpp"
#include "json.hpp"
#include "pyobject.hpp"
#include "wallet_.h"

#include "localize.hpp"

using namespace eosio::client::localize;
using namespace eosio::chain;

uint64_t string_to_uint64_(string str) {
   try {
      return name(str).value;
   } catch (fc::assert_exception& e) {
      elog(e.to_detail_string());
   } catch (fc::exception& e) {
      elog(e.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   }
   return 0;
}

string uint64_to_string_(uint64_t n) {
	return name(n).to_string();
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
       db.head_block_producer(),
       std::bitset<64>(db.get_dynamic_global_properties().recent_slots_filled)
           .to_string(),
       __builtin_popcountll(
           db.get_dynamic_global_properties().recent_slots_filled) /
           64.0};
}

auto tx_expiration = fc::seconds(30);
bool tx_force_unique = false;


PyObject* push_transaction(signed_transaction& trx, bool skip_sign, packed_transaction::compression_type compression = packed_transaction::none) {
   auto info = get_info();
   trx.expiration = info.head_block_time + tx_expiration;
   trx.set_reference_block(info.head_block_id);

   //    transaction_set_reference_block(trx, info.head_block_id);
//   boost::sort(trx.scope);

   if (!skip_sign) {
      sign_transaction(trx);
   }

   auto rw = app().get_plugin<chain_plugin>().get_read_write_api();
   chain_apis::read_write::push_transaction_results result;

   bool success = false;
//   PyThreadState* state = PyEval_SaveThread();
   try {
   		result = rw.push_transaction(fc::variant(packed_transaction(trx, compression)).get_object());
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

PyObject* push_actions(std::vector<chain::action>&& actions, bool skip_sign, packed_transaction::compression_type compression = packed_transaction::none ) {
   signed_transaction trx;
   trx.actions = std::forward<decltype(actions)>(actions);

   return push_transaction(trx, skip_sign, compression);
}

PyObject* send_actions(std::vector<chain::action>&& actions, bool skip_sign, packed_transaction::compression_type compression = packed_transaction::none) {
   return push_actions(std::forward<decltype(actions)>(actions), skip_sign, compression);
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

      return send_actions(std::move(actions), !sign);

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

   string recent_slots =
       std::bitset<64>(db.get_dynamic_global_properties().recent_slots_filled)
           .to_string();
   key = "recent_slots";
   dict.add(key, recent_slots);

   double participation_rate =
       __builtin_popcountll(
           db.get_dynamic_global_properties().recent_slots_filled) /
       64.0;
   key = "participation_rate";
   dict.add(key, participation_rate);

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


string generate_nonce_value() {
   return fc::to_string(fc::time_point::now().time_since_epoch().count());
}

chain::action generate_nonce() {
   auto v = generate_nonce_value();
   variant nonce = fc::mutable_variant_object()
         ("value", v);
   return chain::action( {}, config::system_account_name, "nonce", fc::raw::pack(nonce));
}


PyObject* transfer_(string& sender, string& recipient, int amount, string memo,
                    bool sign) {

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

	return send_actions(std::move(actions), !sign);

}

PyObject* push_message_(string& contract, string& action, string& args, map<string, string>& permissions,
                        bool sign, bool rawargs) {
   signed_transaction trx;
   try {
      //		ilog("Converting argument to binary...");
      auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
      auto rw_api = app().get_plugin<chain_plugin>().get_read_write_api();
      eosio::chain_apis::read_only::abi_json_to_bin_params params;
      if (!rawargs) {
         params = {contract, action, fc::json::from_string(args)};
      } else {
         std::vector<char> v(args.begin(), args.end());
         params = {contract, action, fc::variant(v)};
      }

      vector<chain::permission_level> accountPermissions;
      for (auto it = permissions.begin(); it != permissions.end(); it++) {
         accountPermissions.push_back(chain::permission_level{name(it->first), name(it->second)});
      }

      vector<chain::action> actions;

      if (action.size() == 0) {//evm
      		std::vector<char> v(args.begin(), args.end());
         actions.emplace_back(accountPermissions, contract, action, v);
      } else {
         auto result = ro_api.abi_json_to_bin(params);
         actions.emplace_back(accountPermissions, contract, action, result.binargs);
      }

      if (tx_force_unique) {
         actions.emplace_back( generate_nonce() );
      }

      return send_actions(std::move(actions), !sign);

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

PyObject* set_contract_(string& account, string& wastPath, string& abiPath,
                        int vm_type, bool sign) {
   try {
      std::string wast;
      contracts::setcode handler;
      handler.vmtype = vm_type;
      if (vm_type == 0) {
         std::cout << localized("Reading WAST...") << std::endl;
         fc::read_file_contents(wastPath, wast);

         vector<uint8_t> wasm;
         const string binary_wasm_header = "\x00\x61\x73\x6d";
         if(wast.compare(0, 4, binary_wasm_header) == 0) {
            std::cout << localized("Using already assembled WASM...") << std::endl;
            wasm = vector<uint8_t>(wast.begin(), wast.end());
         }
         else {
            std::cout << localized("Assembling WASM...") << std::endl;
            wasm = assemble_wast(wast);
         }
         handler.account = account;
         handler.code.assign(wasm.begin(), wasm.end());
      } else if (vm_type == 1 || vm_type == 2) {
         fc::read_file_contents(wastPath, wast);
         handler.account = account;
         handler.code.assign(wast.begin(), wast.end());
      }

      vector<chain::action> actions;
      actions.emplace_back( vector<chain::permission_level>{{account,"active"}}, handler);

      if (!abiPath.empty()) {
				contracts::setabi handler;
				handler.account = account;
				try {
					handler.abi = fc::json::from_file(abiPath).as<contracts::abi_def>();
				} EOS_CAPTURE_AND_RETHROW(abi_type_exception,  "Fail to parse ABI JSON")
				actions.emplace_back( vector<chain::permission_level>{{account,"active"}}, handler);
      }

      std::cout << localized("Publishing contract...") << std::endl;
      return send_actions(std::move(actions), !sign, packed_transaction::zlib);

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



