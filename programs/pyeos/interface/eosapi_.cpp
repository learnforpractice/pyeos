#include "eosapi_.hpp"

#include <fc/time.hpp>
#include <fc/io/raw.hpp>
#include <fc/exception/exception.hpp>

#include <eosio/chain/block_summary_object.hpp>
#include <eosio/wallet_plugin/wallet_plugin.hpp>

#include <eosio/utilities/common.hpp>
#include <eosio/chain/wast_to_wasm.hpp>
#include <eosio/chain/contract_types.hpp>
#include <eosio/history_plugin/history_plugin.hpp>
#include <eosio/chain/plugin_interface.hpp>
#include <eosio/chain/symbol.hpp>
#include <eosio/chain/authorization_manager.hpp>
#include <eosio/chain/producer_schedule.hpp>

#include "fc/bitutil.hpp"
#include "json.hpp"
#include "pyobject.hpp"
#include "wallet_.h"

#include <regex>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include <vm_manager.hpp>
#include <chrono>
using namespace std::chrono_literals;

#include <mutex>
#include <condition_variable>

static std::mutex m;
static std::shared_ptr<std::condition_variable> cv(new std::condition_variable());


using namespace eosio;
using namespace eosio::chain;
namespace bio = boost::iostreams;

static auto tx_expiration = fc::seconds(120);
static bool tx_force_unique = false;

static uint32_t tx_max_net_usage = 0;

uint64_t string_to_uint64_(string str) {
   try {
      return name(str).value;
   }  FC_LOG_AND_DROP();
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

chain_plugin& get_chain_plugin() {
   return app().get_plugin<eosio::chain_plugin>();
}

history_plugin& get_history_plugin() {
   return app().get_plugin<eosio::history_plugin>();
}

uint32_t now2_() { return fc::time_point::now().sec_since_epoch(); }

controller& get_controller() { return get_chain_plugin().chain(); }

wallet_plugin& get_wallet_plugin() {
   return app().get_plugin<eosio::wallet_plugin>();
}

wallet_manager& get_wm() {
   return get_wallet_plugin().get_wallet_manager();
}

producer_plugin& get_producer_plugin() {
   return app().get_plugin<eosio::producer_plugin>();
   /*
   abstract_plugin& plugin = app().get_plugin("eosio::producer_plugin");
   return *static_cast<producer_plugin*>(&plugin);
   */
}

static fc::variant json_from_file_or_string(const string& file_or_str, fc::json::parse_type ptype = fc::json::legacy_parser)
{
   return fc::json::from_string(file_or_str, ptype);
}

static authority parse_json_authority(const std::string& authorityJsonOrFile) {
   try {
      return json_from_file_or_string(authorityJsonOrFile).as<authority>();
   } EOS_RETHROW_EXCEPTIONS(authority_type_exception, "Fail to parse Authority JSON '${data}'", ("data",authorityJsonOrFile))
}

static authority parse_json_authority_or_key(const std::string& authorityJsonOrFile) {
   if (boost::istarts_with(authorityJsonOrFile, "EOS")) {
      try {
         return authority(public_key_type(authorityJsonOrFile));
      } EOS_RETHROW_EXCEPTIONS(public_key_type_exception, "Invalid public key: ${public_key}", ("public_key", authorityJsonOrFile))
   } else {
      return parse_json_authority(authorityJsonOrFile);
   }
}

static read_only::get_info_results get_info() {
   auto& ro_api = get_chain_plugin().get_read_only_api();
   eosio::chain_apis::read_only::get_info_params params = {};
   return ro_api.get_info(params);
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

static fc::variant determine_required_keys(const signed_transaction& trx) {
   // TODO better error checking
//   const auto& public_keys = call(wallet_host, wallet_port, wallet_public_keys);
   auto& wallet_mgr = get_wallet_plugin().get_wallet_manager();
//   flat_set<public_key_type> wallet_manager::get_public_keys() {
   const auto& public_keys = wallet_mgr.get_public_keys();

   eosio::chain_apis::read_only::get_required_keys_params get_arg = {fc::variant(trx), public_keys};
//   auto get_arg = fc::mutable_variant_object
//           ("transaction", (transaction)trx)
//           ("available_keys", variant(public_keys));
//   read_only::get_required_keys_result
//   const auto& required_keys = call(host, port, get_required_keys, get_arg);
   auto& ro_api = get_chain_plugin().get_read_only_api();
   auto results = ro_api.get_required_keys(get_arg);
   return fc::variant(results.required_keys);
}

bool gen_transaction(signed_transaction& trx, bool sign, int32_t extra_kcpu = 1000, packed_transaction::compression_type compression = packed_transaction::none) {
   auto info = get_info();

//   trx.expiration = info.head_block_time + tx_expiration;
   trx.expiration = fc::time_point::now() + tx_expiration;

   trx.set_reference_block(info.head_block_id);

   if (tx_force_unique) {
      trx.context_free_actions.emplace_back( generate_nonce() );
   }

   auto required_keys = determine_required_keys(trx);
   size_t num_keys = required_keys.is_array() ? required_keys.get_array().size() : 1;

//   trx.max_kcpu_usage = (tx_max_cpu_usage + 1023)/1024;
   trx.max_net_usage_words = (tx_max_net_usage + 7)/8;

   if (sign) {
      sign_transaction(trx);
   }
   return true;
}

fc::variant push_transaction( signed_transaction& trx, int32_t extra_kcpu = 1000, packed_transaction::compression_type compression = packed_transaction::none );
fc::variant cleos_push_transaction( signed_transaction& trx, packed_transaction::compression_type compression = packed_transaction::none );

PyObject* push_transactions__(vector<vector<chain::action>>& vv, bool sign, uint64_t skip_flag, bool async, bool compress, int32_t max_ram_usage) {
   packed_transaction::compression_type compression;
   if (compress) {
      compression = packed_transaction::zlib;
   } else {
      compression = packed_transaction::none;
   }

   uint64_t cost_time = 0;
   PyArray _outputs;

   try {
      for (auto& v: vv) {
         signed_transaction trx;
         trx.expiration = fc::time_point::now() + tx_expiration;
         trx.max_ram_usage = max_ram_usage;
         for(auto& action: v) {
            trx.actions.emplace_back(std::move(action));
         }
         variant result;
         Py_BEGIN_ALLOW_THREADS
         try {
            result = push_transaction(trx);
         }FC_LOG_AND_DROP();
         Py_END_ALLOW_THREADS
         PyObject* o = python::json::to_string(result["processed"]);
         _outputs.append(o);
         Py_DECREF(o);
      }
   }  FC_LOG_AND_DROP();

   PyArray res;

   PyObject* o = _outputs.get();
   res.append(o);
   Py_DECREF(o);

   res.append(py_new_uint64(0));
   return res.get();
}


PyObject* push_transaction_async_(std::shared_ptr<packed_transaction> ppt) {
   bool ready = false;
   fc::mutable_variant_object output;

   uint64_t start = get_microseconds();

   appbase::app().get_io_service().post([&](){
      app().get_method<plugin_interface::incoming::methods::transaction_async>()(ppt, true, [&](const fc::static_variant<fc::exception_ptr, transaction_trace_ptr>& result) mutable -> void{
         if (result.contains<fc::exception_ptr>()) {
            string s = result.get<fc::exception_ptr>()->to_string();
            output("except", s);
         } else {
            auto trx_trace_ptr = result.get<transaction_trace_ptr>();
            auto v = get_controller().to_variant_with_abi(*trx_trace_ptr, fc::microseconds(300*1000));
            output = v.get_object();
            output("except", "");
         }
         {
            std::lock_guard<std::mutex> lk(m);
            ready = true;
         }
         cv->notify_one();
      });
   });

   Py_BEGIN_ALLOW_THREADS
   std::unique_lock<std::mutex> lk(m);
#if 0
   if (!cv->wait_for(lk, 1000ms, [&]{return ready;})) {
      if (!get_chain_plugin().chain().pending_block_state()) {
         output("except", "not in pending block state, ");
      }
   }
#else
   cv->wait(lk, [&]{return ready;});
   if (get_chain_plugin().chain().pending_block_state()) {
   } else {
      output("except", "not in pending block state, ");
   }
#endif
   Py_END_ALLOW_THREADS
//   wlog("+++++++++++++push_transaction_async: ${n}", ("n", get_microseconds()-start));
   return python::json::to_string(output);
}

PyObject* push_transactions_(vector<vector<chain::action>>& vv, bool sign, uint64_t skip_flag, bool async, bool compress, int32_t max_ram_usage) {
   packed_transaction::compression_type compression;
   if (compress) {
      compression = packed_transaction::zlib;
   } else {
      compression = packed_transaction::none;
   }

   uint64_t cost_time = 0;
   PyArray _outputs;

   try {
      for (auto& v: vv) {
         signed_transaction trx;
         trx.max_ram_usage = max_ram_usage;
         for(auto& a: v) {
            trx.actions.emplace_back(std::move(a));
         }
         gen_transaction(trx, sign, 10000000, compression);

         if (async) {
            PyObject* result;
            variant v;
            uint64_t cost = get_microseconds();
#if 0
            std::shared_ptr<packed_transaction> ppt(new packed_transaction(std::move(trx), compression));
            result = push_transaction_async_(ppt);
#else
            Py_BEGIN_ALLOW_THREADS
            try {
               v = cleos_push_transaction( trx, compression );
            } FC_LOG_AND_DROP();
//            wlog("+++++++++++++++cleos_push_transaction end!");
            Py_END_ALLOW_THREADS
            result = python::json::to_string(v);
#endif
            cost_time += (get_microseconds() - cost);
            _outputs.append(result);
            Py_DECREF(result);
         } else {
            std::shared_ptr<packed_transaction> ppt(new packed_transaction(std::move(trx), compression));
            auto mtrx = std::make_shared<transaction_metadata>(*ppt);
             controller& ctrl = get_chain_plugin().chain();
             uint32_t cpu_usage = ctrl.get_global_properties().configuration.min_transaction_cpu_usage;

             auto deadline = fc::time_point::now() + fc::milliseconds(100);
             uint64_t cost = get_microseconds();
             auto trx_trace_ptr = ctrl.push_transaction(mtrx, deadline, cpu_usage);
             cost_time += (get_microseconds() - cost);

             fc::variant pretty_output = ctrl.to_variant_with_abi( *trx_trace_ptr, fc::microseconds(30*1000) );
             PyObject* o = python::json::to_string(pretty_output);
             _outputs.append(o);
             Py_DECREF(o);
         }
      }
   }  FC_LOG_AND_DROP();

   PyArray res;

   PyObject* o = _outputs.get();
   res.append(o);
   Py_DECREF(o);

   res.append(py_new_uint64(cost_time));
   return res.get();
}


PyObject* gen_transaction_(vector<chain::action>& v, int expiration) {
   packed_transaction::compression_type compression = packed_transaction::none;

   uint64_t cost_time = 0;

   try {
      signed_transaction trx;
      for(auto& action: v) {
         trx.actions.push_back(std::move(action));
      }

      auto info = get_info();
      trx.expiration = fc::time_point::now() + fc::seconds(expiration);;
      trx.set_reference_block(info.head_block_id);

//      trx.max_kcpu_usage = (tx_max_cpu_usage + 1023)/1024;
      trx.max_net_usage_words = (tx_max_net_usage + 7)/8;
      return python::json::to_string(fc::variant(trx));

   } FC_LOG_AND_DROP();

   return py_new_none();
}

PyObject* sign_transaction_(string& trx_json_to_sign, string& str_private_key) {
   try {
      signed_transaction trx = python::json::from_string(trx_json_to_sign).as<signed_transaction>();

      auto priv_key = fc::crypto::private_key::regenerate(*utilities::wif_to_key(str_private_key));
      trx.sign(priv_key, get_info().chain_id);
      return python::json::to_string(fc::variant(trx));

   } FC_LOG_AND_DROP();
   return py_new_none();

}

PyObject* push_raw_transaction_(string& signed_trx) {
   bool success = false;

   auto& rw = get_chain_plugin().get_read_write_api();
   chain_apis::read_write::push_transaction_results result;

   try {
      signed_transaction trx = python::json::from_string(signed_trx).as<signed_transaction>();
      packed_transaction::compression_type compression = packed_transaction::none;

      auto pt = packed_transaction(std::move(trx), compression);
      auto mtrx = std::make_shared<transaction_metadata>(pt);

      controller& ctrl = get_chain_plugin().chain();
      uint32_t cpu_usage = ctrl.get_global_properties().configuration.min_transaction_cpu_usage;
      auto trx_trace_ptr = ctrl.push_transaction(mtrx, fc::time_point::maximum(), cpu_usage);
      return python::json::to_string(trx_trace_ptr);
   }  FC_LOG_AND_DROP();
   return py_new_none();
}

int produce_block_() {
//   return get_producer_plugin().produce_block();
   return 0;
}

int produce_block_start_() {
   try {
      return get_producer_plugin().produce_block_start();
   } FC_LOG_AND_DROP();
   return 0;
}

int produce_block_end_() {
   try {
      return get_producer_plugin().produce_block_end();
   } FC_LOG_AND_DROP();
   return 0;
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
   private_key_type priv(wif_key);

   string pub_key = string(priv.get_public_key());
   return py_new_string(pub_key);
}

void modify_permission( const permission_object& permission, const authority& auth ) {
   get_controller().db().modify( permission, [&](permission_object& po) {
      po.auth = auth;
      po.last_updated = fc::time_point::now();
   });
}

bool update_permission_(uint64_t account, const string& owner, const string& active) {
   auto owner_auth = eosio::chain::authority{1, {{public_key_type(owner), 1}}, {}};
   auto active_auth = eosio::chain::authority{1, {{public_key_type(active), 1}}, {}};

   auto& authorization = get_controller().get_mutable_authorization_manager();

   auto owner_perm = authorization.find_permission(eosio::chain::permission_level{account, eosio::chain::name("owner")});
   auto active_perm = authorization.find_permission(eosio::chain::permission_level{account, eosio::chain::name("active")});

   modify_permission( *owner_perm, owner_auth );
   modify_permission( *active_perm, active_auth );
   return true;
}

bool set_proposed_producers_(string& producer, string& public_key) {
   vector<producer_key> producers;
   auto key = eosio::chain::producer_key{producer, public_key_type(public_key)};
   producers.push_back(key);

   producer_schedule_type sch;
   sch.producers = std::move(producers);

   const auto& gpo = get_controller().get_global_properties();
   get_controller().db().modify( gpo, [&]( auto& gp ) {
      gp.proposed_schedule_block_num = 12149;
      gp.proposed_schedule = std::move(sch);
   });
   return true;
}

PyObject* create_account_(string creator, string newaccount, string owner,
                          string active, int sign) {
   try {
      auto owner_auth = eosio::chain::authority{1, {{public_key_type(owner), 1}}, {}};
      auto active_auth = eosio::chain::authority{1, {{public_key_type(active), 1}}, {}};
      auto recovery_auth = eosio::chain::authority{1, {}, {{{creator, "active"}, 1}}};

      vector<vector<chain::action>> vv;
      vector<chain::action> actions;
      actions.emplace_back( vector<chain::permission_level>{{creator,"active"}},
                                eosio::chain::newaccount{
                                        .creator      = creator,
                                        .name         = newaccount,
                                        .owner        = owner_auth,
                                        .active       = active_auth
                                }
      );
      vv.emplace_back(std::move(actions));
      return push_transactions_(vv, sign);

   }  FC_LOG_AND_DROP();

   return py_new_none();
}

PyObject* get_info_() {
   auto& ro_api = get_chain_plugin().get_read_only_api();
   chain_apis::read_only::get_info_params params = {};
   chain_apis::read_only::get_info_results results = ro_api.get_info(params);
   return python::json::to_string(results);

   PyDict dict;
   const controller& db = get_controller();
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
   auto& ro_api = get_chain_plugin().get_read_only_api();
   try {
      chain_apis::read_only::get_block_params params = {string(num_or_id)};
      auto results = ro_api.get_block(params);
      return python::json::to_string(results);
   }  FC_LOG_AND_DROP();

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
   try {
      auto& ro_api = get_chain_plugin().get_read_only_api();

      eosio::chain_apis::read_only::get_account_results result;
      eosio::chain_apis::read_only::get_account_params params = {chain::name(_name).value};
      result = ro_api.get_account(params);
//      string s = json::json::to_string(result);
      return python::json::to_string(result);
   }  catch(...) {
   }

   return py_new_none();
}

bool is_account_(const char* _name) {
   return db_api::get().is_account(eosio::chain::name(_name));
}

PyObject* get_currency_balance_(string& _code, string& _account, string& _symbol) {
   try {
      PyArray arr;

      auto& ro_api = get_chain_plugin().get_read_only_api();

      eosio::chain_apis::read_only::get_currency_balance_params params = {chain::name(_code), chain::name(_account), _symbol};
      auto result = ro_api.get_currency_balance(params);
      if (result.size() > 0) {
         arr.append((uint64_t)result[0].get_amount());
         arr.append((uint64_t)result[0].get_symbol().value());
         return arr.get();
      }
   } catch (...) {

   }
   return py_new_none();
//   return python::json::to_string(result);
}

PyObject* get_actions_(uint64_t account, int pos, int offset) {

   eosio::history_apis::read_only::get_actions_params params = {
       name(account),
       pos,
       offset
   };
   try {
      auto& ro_api = get_history_plugin().get_read_only_api();
      auto results = ro_api.get_actions(params);
      return python::json::to_string(fc::variant(results));
   }  FC_LOG_AND_DROP();

   return py_new_none();
}


PyObject* get_transaction_(string& id) {

   eosio::history_apis::read_only::get_transaction_params params = {
       chain::transaction_id_type(id)
   };
   try {
      auto& ro_api = get_history_plugin().get_read_only_api();
      auto results = ro_api.get_transaction(params);
      return python::json::to_string(fc::variant(results));
   }  FC_LOG_AND_DROP();

   return py_new_none();
}

int get_code_(string& name, string& wast, string& str_abi, string& code_hash, int& vm_type) {
   try {
      controller& db = get_chain_plugin().chain();

      chain_apis::read_only::get_code_results result;
      result.account_name = name;
      const auto& d = db.db();
      const auto& accnt = d.get<account_object, by_name>(name);

      if (accnt.code.size()) {
         if (accnt.vm_type == 0) {
            result.wast = chain::wasm_to_wast((uint8_t*)accnt.code.data(), accnt.code.size(), true);
         } else {
            result.wast = string((const char*)accnt.code.data(), accnt.code.size());
         }
         result.code_hash = fc::sha256::hash(accnt.code.data(), accnt.code.size());
      }

      chain::abi_def abi;
      if( chain::abi_serializer::to_abi(accnt.abi, abi) ) {
         result.abi = std::move(abi);
      }

      vm_type = accnt.vm_type;
      wast = result.wast;
      code_hash = result.code_hash.str();
      str_abi = fc::json::to_string(fc::variant(result.abi));
//      abi = fc::json::to_string(get_abi_serializer().binary_to_variant("abi_def", accnt.abi));
//      abi = fc::json::to_string(result.abi);
      return 0;
   }  FC_LOG_AND_DROP();

   return -1;
}

void get_code_hash_(string& name, string& code_hash) {
   try {
      controller& db = get_chain_plugin().chain();

      chain_apis::read_only::get_code_results result;
      result.account_name = name;
      const auto& d = db.db();
      const auto& accnt = d.get<account_object, by_name>(name);

      if (accnt.code.size()) {
         code_hash = fc::sha256::hash(accnt.code.data(), accnt.code.size());
      }
   }  FC_LOG_AND_DROP();

   return;
}

uint64_t get_code_update_time_ms_(string& name) {
   try {
      controller& ctrl = get_chain_plugin().chain();
      const auto& a = ctrl.db().get<account_object, by_name>(name);
      return a.last_code_update.time_since_epoch().count()/1000;
   }  FC_LOG_AND_DROP();
   return 0;
}

int get_table_(string& scope, string& code, string& table, string& result) {
   try {
      auto& ro_api = get_chain_plugin().get_read_only_api();
      chain_apis::read_only::get_table_rows_params params;
      params.json = true;
      params.scope = scope;
      params.code = code;
      params.table = table;
      chain_apis::read_only::get_table_rows_result results =
          ro_api.get_table_rows(params);
      result = fc::json::to_string(results);
      return 0;
   }  FC_LOG_AND_DROP();
   return -1;
}

void wast2wasm_(string& wast, string& result) {
   try {
      auto wasm = wast_to_wasm(wast);
      result = string((char *)wasm.data(), wasm.size());
   } catch (...) {
      elog("wast_to_wasm failed");
   }
}

void wasm2wast_(string& wasm, string& result) {
   try {
      auto wast = wasm_to_wast((uint8_t*)wasm.c_str(), wasm.size(), true);
      result = string((char *)wast.data(), wast.size());
   } catch (...) {
      elog("wast_to_wasm failed");
   }
}


bool is_replay_() {
   return get_chain_plugin().is_replay();
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

void fc_pack_setcode_(chain::setcode _setcode, vector<char>& out) {
   out = fc::raw::pack(_setcode);
}


void fc_pack_setabi_(string& abiPath, uint64_t account, string& out) {
   try {
      setabi handler;
      handler.account = account;
      handler.abi = fc::raw::pack(fc::json::from_file(abiPath).as<abi_def>());
      auto _out = fc::raw::pack(handler);
      out = string(_out.begin(), _out.end());
   } FC_LOG_AND_DROP();
}

void fc_pack_setconfig_(string& abiPath, uint64_t account, string& out) {
   try {
      setconfig handler;
      handler.account = account;
      handler.config = fc::raw::pack(fc::json::from_file(abiPath).as<abi_def>());
      auto _out = fc::raw::pack(handler);
      out = string(_out.begin(), _out.end());
   } FC_LOG_AND_DROP();
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
   eosio::chain::updateauth _updateauth = {_account, _permission, _parent, auth};
   vector<char> v = fc::raw::pack(_updateauth);
   result = string(v.data(), v.size());
}

void fc_pack_args(uint64_t code, uint64_t action, string& json, string& bin) {
   auto& ro_api = get_chain_plugin().get_read_only_api();
   eosio::chain_apis::read_only::abi_json_to_bin_params params;
   params = {code, action, fc::json::from_string(json)};
   try {
      auto result = ro_api.abi_json_to_bin(params);
      bin = string(result.binargs.data(), result.binargs.size());
   } FC_LOG_AND_DROP();
}

PyObject* fc_unpack_args(uint64_t code, uint64_t action, string& bin) {
   auto& ro_api = get_chain_plugin().get_read_only_api();
   eosio::chain_apis::read_only::abi_bin_to_json_params params;
   params = {code, action, vector<char>(bin.begin(), bin.end())};
   try {
      auto result = ro_api.abi_bin_to_json(params);
      return python::json::to_string(result.args);
   } FC_LOG_AND_DROP();
   return py_new_none();
}

void n_to_symbol_(uint64_t n, string& out) {
   try {
      out = eosio::chain::symbol(n).name();
   } FC_LOG_AND_DROP();
}

uint64_t symbol_to_n_(string& n) {
   try {
      return eosio::chain::symbol::from_string(n).value();
   } FC_LOG_AND_DROP();
   return 0;
}

template<size_t Limit>
struct read_limiter {
   using char_type = char;
   using category = bio::multichar_output_filter_tag;

   template<typename Sink>
   size_t write(Sink &sink, const char* s, size_t count)
   {
      EOS_ASSERT(_total + count <= Limit, tx_decompression_error, "Exceeded maximum decompressed transaction size");
      _total += count;
      return bio::write(sink, s, count);
   }

   size_t _total = 0;
};

bool debug_mode_() {
   return appbase::app().debug_mode();
}

void sha256_(string& data, string& hash) {
   hash = fc::sha256::hash(data.c_str(), data.size()).str();
}

#include <eosiolib_native/vm_api.h>

uint64_t ethaddr2n_(string& addr) {
   return get_vm_api()->ethaddr2n((char*)addr.c_str(), addr.size());
}

void n2ethaddr_(uint64_t n, string& addr) {
   char _addr[20];
   get_vm_api()->n2ethaddr(n, _addr, sizeof(_addr));
   addr = string(_addr, sizeof(_addr));
}

#include <boost/signals2/connection.hpp>
boost::signals2::connection cnn;
boost::signals2::connection cnn2;

void transaction_listen_() {
   cnn = get_chain_plugin().chain().accepted_transaction.connect( [&]( const chain::transaction_metadata_ptr& t ) {
      vmdlog("got accepted transaction!");
   } );

   cnn2 = get_chain_plugin().chain().applied_transaction.connect( [&]( const chain::transaction_trace_ptr& t ) {
      vmdlog("got applied transaction!");
   } );

}

void transaction_disconnect_() {
   cnn.disconnect();
   cnn2.disconnect();
}

void get_active_producers_(vector<string>& producers) {
   auto aps = get_controller().active_producers();
   for(const auto& producer : aps.producers ) {
      producers.push_back(producer.producer_name.to_string());
   }
}

