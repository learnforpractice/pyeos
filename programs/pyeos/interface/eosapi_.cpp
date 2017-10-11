#include "eosapi_.hpp"

#include "pyobject.hpp"
#include "wallet_.h"
#include "fc/bitutil.hpp"
#include "eos/chain/block_summary_object.hpp"
#include "json.hpp"
#include <fc/time.hpp>

void quit_app_() {
   app().quit();
}
uint32_t now2_(){
    return fc::time_point::now().sec_since_epoch();
}

chain_controller& get_db(){
    return app().get_plugin<chain_plugin>().chain();
}

wallet_manager& get_wm(){
   return app().get_plugin<wallet_plugin>().get_wallet_manager();
}

inline std::vector<Name> sort_names( std::vector<Name>&& names ) {
   std::sort( names.begin(), names.end() );
   auto itr = std::unique( names.begin(), names.end() );
   names.erase( itr, names.end() );
   return names;
}

vector<uint8_t> assemble_wast( const std::string& wast ) {
   IR::Module module;
   std::vector<WAST::Error> parseErrors;
   WAST::parseModule(wast.c_str(),wast.size(),module,parseErrors);
   if(parseErrors.size())
   {
      // Print any parse errors;
      std::cerr << "Error parsing WebAssembly text file:" << std::endl;
      for(auto& error : parseErrors)
      {
         std::cerr << ":" << error.locus.describe() << ": " << error.message.c_str() << std::endl;
         std::cerr << error.locus.sourceLine << std::endl;
         std::cerr << std::setw(error.locus.column(8)) << "^" << std::endl;
      }
      FC_ASSERT( !"error parsing wast" );
   }

   try
   {
      // Serialize the WebAssembly module.
      Serialization::ArrayOutputStream stream;
      WASM::serialize(stream,module);
      return stream.getBytes();
   }
   catch(Serialization::FatalSerializationException& exception)
   {
      std::cerr << "Error serializing WebAssembly binary file:" << std::endl;
      std::cerr << exception.message << std::endl;
      throw;
   }
}

read_only::get_info_results get_info() {
   auto& db = get_db();
	return {
		db.head_block_num(),
		db.last_irreversible_block_num(),
		db.head_block_id(),
		db.head_block_time(),
		db.head_block_producer(),
		std::bitset<64>(db.get_dynamic_global_properties().recent_slots_filled).to_string(),
		__builtin_popcountll(db.get_dynamic_global_properties().recent_slots_filled) / 64.0
	};
}

string push_transaction_bk( SignedTransaction& trx, bool sign ) {
    auto info = get_info();
    trx.expiration = info.head_block_time + 100; //chain.head_block_time() + 100;
//    get_db().get_database().get<block_summary_object>((uint16_t)trx.refBlockNum).block_id;

    trx.refBlockNum = fc::endian_reverse_u32(info.head_block_id._hash[0]);
    trx.refBlockPrefix = info.head_block_id._hash[1];

//    transaction_set_reference_block(trx, info.head_block_id);
    boost::sort( trx.scope );

    if (sign) {
       sign_transaction(trx);
    }

    auto v = fc::variant(trx);
//	ilog(fc::json::to_string( trx ));

	auto rw = app().get_plugin<chain_plugin>().get_read_write_api();
	auto result = fc::json::to_string(rw.push_transaction(v.get_object()));
//	ilog(result);
	return result;


    ProcessedTransaction pts = get_db().push_transaction(trx,sign);
    return get_db().transaction_to_variant(pts).get_string();
}

PyObject* push_transaction( SignedTransaction& trx, bool sign ) {
    auto info = get_info();
    trx.expiration = info.head_block_time + 100; //chain.head_block_time() + 100;
//    get_db().get_database().get<block_summary_object>((uint16_t)trx.refBlockNum).block_id;

    trx.refBlockNum = fc::endian_reverse_u32(info.head_block_id._hash[0]);
    trx.refBlockPrefix = info.head_block_id._hash[1];

//    transaction_set_reference_block(trx, info.head_block_id);
    boost::sort( trx.scope );

    if (sign) {
       sign_transaction(trx);
    }

   auto rw = app().get_plugin<chain_plugin>().get_read_write_api();
   chain_apis::read_write::push_transaction_results result;

   bool success = false;
   PyThreadState* state = PyEval_SaveThread();
   try{
      result = rw.push_transaction(fc::variant(trx).get_object());
      success = true;
   }catch(fc::assert_exception& e){
      elog(e.to_detail_string());
   }catch(fc::exception& e){
      elog(e.to_detail_string());
   }catch(boost::exception& ex){
      elog(boost::diagnostic_information(ex));
   }

   PyEval_RestoreThread(state);

   if(success){
      return python::json::to_string(result);
   }
   return py_new_none();
}

PyObject* create_key_(){
	auto priv_ = fc::ecc::private_key::generate();
	auto pub_ = public_key_type( priv_.get_public_key() );
	PyDict dict;
	string key;
	string value;
	value = string(pub_);
	key = "public";
	dict.add(key,value);

	key = "private";
	value = string(key_to_wif(priv_.get_secret()));
	dict.add(key,value);
	return dict.get();
}

PyObject *get_public_key_(string& wif_key){
   auto priv_key = eos::utilities::wif_to_key(wif_key);
   if (!priv_key){
      return py_new_none();
   }
   string pub_key = string(public_key_type(priv_key->get_public_key()));
   return py_new_string(pub_key);
}

PyObject* create_account_(string creator, string newaccount, string owner, string active, int sign) {
   try {
      auto owner_auth   = eos::chain::Authority{1, {{public_key_type(owner), 1}}, {}};
      auto active_auth  = eos::chain::Authority{1, {{public_key_type(active), 1}}, {}};
      auto recovery_auth = eos::chain::Authority{1, {}, {{{creator, "active"}, 1}}};

      uint64_t deposit = 1;
      SignedTransaction trx;
      trx.scope = sort_names({creator,config::EosContractName});
      transaction_emplace_message(trx, config::EosContractName, vector<types::AccountPermission>{{creator,"active"}}, "newaccount",
                                 types::newaccount{creator, newaccount, owner_auth,
                                              active_auth, recovery_auth, deposit});
      return push_transaction(trx, sign);
   }catch(fc::assert_exception& e){
      elog(e.to_detail_string());
   }catch(fc::exception& e){
      elog(e.to_detail_string());
   }catch(boost::exception& ex){
      elog(boost::diagnostic_information(ex));
   }
   return py_new_none();
}

PyObject* get_info_(){
   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
   chain_apis::read_only::get_info_params params = {};
   chain_apis::read_only::get_info_results results = ro_api.get_info(params);
   return python::json::to_string(results);

   PyDict dict;
   const chain_controller& db=get_db();
   string key;
   string value;

   key = "head_block_num";
   dict.add(key,db.head_block_num());

   key = "last_irreversible_block_num";
   dict.add(key,db.last_irreversible_block_num());

   key = "head_block_id";
   value = db.head_block_id().str();
   dict.add(key,value);

   key = "head_block_time";
   value = db.head_block_time().to_iso_string();
   dict.add(key,value);

   key = "head_block_producer";
   value = db.head_block_producer().toString();
   dict.add(key,value);

   string recent_slots = std::bitset<64>(db.get_dynamic_global_properties().recent_slots_filled).to_string();
   key = "recent_slots";
   dict.add(key,recent_slots);

   double participation_rate = __builtin_popcountll(db.get_dynamic_global_properties().recent_slots_filled) / 64.0;
   key = "participation_rate";
   dict.add(key,participation_rate);

   return dict.get();


}
/*
{
  "previous": "0000000000000000000000000000000000000000000000000000000000000000",
  "timestamp": "2017-09-02T13:38:39",
  "transaction_merkle_root": "0000000000000000000000000000000000000000000000000000000000000000",
  "producer": "initr",
  "producer_changes": [],
  "producer_signature": "1f6a8a50b6993bfa07d0b195c4e5378d4f105e0c6cfe6beed5050948531b7000ef2d75f463d7a095d5b931c3923a78e04406308d2cbd2a5435422046363f948940",
  "cycles": [],
  "id": "00000001b107f27f37bb944c10952a758dc44129fe0310b7a1b76d47a67ad908",
  "block_num": 1,
  "refBlockPrefix": 1284815671
}
*/

PyObject *get_block_(char *num_or_id){
   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
   try{
      chain_apis::read_only::get_block_params params = {string(num_or_id)};
      chain_apis::read_only::get_block_results results = ro_api.get_block(params);
      return python::json::to_string(results);
   } catch (fc::bad_cast_exception& ex) {/* do nothing */}
   catch ( const fc::exception& e ) {
     elog((e.to_detail_string()));
   }catch(boost::exception& ex){
      elog(boost::diagnostic_information(ex));
   }
   return py_new_none();

   PyArray arr;
   auto& db = get_db();

   string block_num_or_id(num_or_id);
   try {
      if (auto block = db.fetch_block_by_id(fc::json::from_string(block_num_or_id).as<chain::block_id_type>())){
         eos::chain_apis::read_only::get_block_results results = *block;

         arr.append(results.previous.str());
         arr.append(results.timestamp.to_iso_string());
         arr.append(results.transaction_merkle_root.str());
         arr.append(results.producer.toString());
         //producer_changes
         fc::variant v;
         fc::to_variant<unsigned char,65>(results.producer_signature,v);
         arr.append(fc::json::to_string(v));
         //cycles
         arr.append(results.id.str());
         arr.append(results.block_num);
         arr.append(results.refBlockPrefix);
      }

      if (auto block = db.fetch_block_by_number(fc::to_uint64(block_num_or_id))){
         eos::chain_apis::read_only::get_block_results results = *block;

         arr.append(results.previous.str());
         arr.append(results.timestamp.to_iso_string());
         arr.append(results.transaction_merkle_root.str());
         arr.append(results.producer.toString());
         //producer_changes
         fc::variant v;
         fc::to_variant<unsigned char,65>(results.producer_signature,v);
         arr.append(fc::json::to_string(v));
         //cycles
         arr.append(results.id.str());
         arr.append(results.block_num);
         arr.append(results.refBlockPrefix);
      }
   } catch (fc::bad_cast_exception& ex) {/* do nothing */}
   catch ( const fc::exception& e ) {
     elog((e.to_detail_string()));
   }catch(boost::exception& ex){
      elog(boost::diagnostic_information(ex));
   }
   return arr.get();
}

/*
   struct get_account_results {
      Name                       name;
      uint64_t                   eos_balance       = 0;
      uint64_t                   staked_balance    = 0;
      uint64_t                   unstaking_balance = 0;
      fc::time_point_sec         last_unstaking_time;
      optional<producer_info>    producer;
      optional<types::Abi>       abi;
   };
*/
PyObject* get_account_(char *name){
   using namespace native::eos;
   PyArray arr;
   PyDict dict;
//   arr.append(name);
/*
   Name                       name;
   Asset                      eos_balance = Asset(0,EOS_SYMBOL);
   Asset                      staked_balance;
   Asset                      unstaking_balance;
   fc::time_point_sec         last_unstaking_time;
   vector<permission>         permissions;
   optional<producer_info>    producer;
*/


   try{
      auto& db = get_db();
      eos::chain_apis::read_only::get_account_results result;
      eos::chain_apis::read_only::get_account_params params = {name};
      string key;
      string value;
      result.name = params.name;
      const auto& d = db.get_database();
      const auto& accnt          = d.get<account_object,by_name>( params.name );
      const auto& balance        = d.get<BalanceObject,byOwnerName>( params.name );
      const auto& staked_balance = d.get<StakedBalanceObject,byOwnerName>( params.name );
      key = "name";
      value = name;
      dict.add(key,value);

      key = "balance";
      dict.add(key,(uint64_t)balance.balance);

      key = "stakedBalance";
      dict.add(key,(uint64_t)staked_balance.stakedBalance);

      key = "unstakingBalance";
      dict.add(key,(uint64_t)staked_balance.unstakingBalance);

      key = "lastUnstakingTime";
      value = staked_balance.lastUnstakingTime.to_iso_string();
      dict.add(key,value);

      if( accnt.abi.size() > 4 ) {
         eos::types::Abi abi;
         fc::datastream<const char*> ds( accnt.abi.data(), accnt.abi.size() );
         fc::raw::unpack( ds, abi );
         string value = fc::json::to_string(fc::variant(abi));
         key = "abi";
         dict.add(key,value);
      }
   }catch(fc::exception& ex){
      elog(ex.to_detail_string());
   }catch(boost::exception& ex){
      elog(boost::diagnostic_information(ex));
   }
   return dict.get();
}

PyObject* get_accounts_(char *public_key){
	PyArray arr;
	try{
		if (public_key == NULL){
			return arr.get();
		}
		auto ro_api = app().get_plugin<account_history_plugin>().get_read_only_api();
		auto rw_api = app().get_plugin<account_history_plugin>().get_read_write_api();
		eos::account_history_apis::read_only::get_key_accounts_params params = {chain::public_key_type{}};
		eos::account_history_apis::read_only::get_key_accounts_results results= ro_api.get_key_accounts(params);

		for(auto it = results.account_names.begin();it!=results.account_names.end();++it){
			arr.append(string(*it));
		}
	}catch(fc::exception& ex){
		elog(ex.to_detail_string());
	}
	return arr.get();
}

PyObject* get_controlled_accounts_(char *account_name){
	PyArray arr;
	try{
		if (account_name == NULL){
			return arr.get();
		}
		auto ro_api = app().get_plugin<account_history_plugin>().get_read_only_api();
		eos::account_history_apis::read_only::get_controlled_accounts_params params = {Name(account_name)};
		eos::account_history_apis::read_only::get_controlled_accounts_results results = ro_api.get_controlled_accounts(params);
		for(auto it = results.controlled_accounts.begin();it!=results.controlled_accounts.end();it++){
			arr.append(string(*it));
		}
	}catch(fc::exception& ex){
		elog(ex.to_detail_string());
	}
	return arr.get();
}

int get_transaction_(string& id,string& result){
	eos::account_history_apis::read_only::get_transaction_params params = {chain::transaction_id_type(id)};
	try{
		auto ro_api = app().get_plugin<account_history_plugin>().get_read_only_api();
		eos::account_history_apis::read_only::get_transaction_results results = ro_api.get_transaction(params);
		result = fc::json::to_string(fc::variant(results));
		return 0;
	}catch(fc::exception& ex){
		elog(ex.to_detail_string());
	}
	return -1;
}

int get_transactions_(string& account_name,int skip_seq,int num_seq,string& result){
	try{
		const eos::account_history_apis::read_only::get_transactions_params params = {
				chain::AccountName(account_name),
				skip_seq,
				num_seq
		};
		auto ro_api = app().get_plugin<account_history_plugin>().get_read_only_api();
		eos::account_history_apis::read_only::get_transactions_results results = ro_api.get_transactions(params);
		result = fc::json::to_string(results);
		return 0;
	}catch(fc::exception& ex){
		elog(ex.to_detail_string());
	}
	return -1;
}

PyObject* transfer_(string& sender,string& recipient,int amount,string memo,bool sign){
	try{
		auto rw_api = app().get_plugin<chain_plugin>().get_read_write_api();
		SignedTransaction trx;
		trx.scope = sort_names({sender,recipient});
		transaction_emplace_message(trx, config::EosContractName,
											vector<types::AccountPermission>{{sender,"active"}},
											"transfer", types::transfer{sender, recipient, amount, memo});
		return push_transaction(trx,sign);
	}catch(fc::exception& ex){
		elog(ex.to_detail_string());
	}
	return py_new_none();
}

PyObject* push_message_(string& contract,string& action,string& args,vector<string> scopes,map<string,string>& permissions,bool sign){
   SignedTransaction trx;
   ilog(args);
   try{
//		ilog("Converting argument to binary...");
		auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
		auto rw_api = app().get_plugin<chain_plugin>().get_read_write_api();

		eos::chain_apis::read_only::abi_json_to_bin_params params = {contract,action,fc::json::from_string(args)};
		auto result = ro_api.abi_json_to_bin(params);

		vector<types::AccountPermission> accountPermissions;
		for (auto it=permissions.begin();it!=permissions.end();it++){
				accountPermissions.push_back(types::AccountPermission(Name(it->first),it->second));
		}
		transaction_emplace_serialized_message(trx, contract, action, accountPermissions,result.binargs);
		for( const auto& s : scopes ) {
		   trx.scope.emplace_back(s);
		}
		return push_transaction(trx,sign);
	}catch(fc::exception& ex){
		elog(ex.to_detail_string());
	}catch(boost::exception& ex){
      elog(boost::diagnostic_information(ex));
   }
/*
   auto obj = get_db().get_database().get<eos::chain::block_summary_object>((uint16_t)trx.refBlockNum);
   ilog("obj.block_id._hash[0] ${n} ", ("n", fc::endian_reverse_u32(obj.block_id._hash[0])));
   ilog("trx.refBlockNum ${n} ", ("n", trx.refBlockNum) );
*/
   return py_new_none();
}

PyObject* set_contract_(string& account,string& wastPath,string& abiPath,int vmtype,bool sign){
	try{
      types::setcode handler;
		std::string wast;
		if (vmtype == 0) {
         std::cout << "Reading WAST..." << std::endl;
         fc::read_file_contents(wastPath, wast);
         std::cout << "Assembling WASM..." << std::endl;
         auto wasm = assemble_wast(wast);
         handler.code.assign(wasm.begin(), wasm.end());
		} else if (vmtype == 1) {
         fc::read_file_contents(wastPath, wast);
		   handler.code.assign(wast.begin(), wast.end());
		}

      handler.account = account;
      handler.vmtype = vmtype;
      handler.abi = fc::json::from_file(abiPath).as<types::Abi>();

		SignedTransaction trx;
		trx.scope = sort_names({config::EosContractName, account});
		transaction_emplace_message(trx, config::EosContractName, vector<types::AccountPermission>{{account,"active"}},
											 "setcode", handler);
		std::cout << "Publishing contract..." << std::endl;
		return push_transaction(trx,sign);
	}catch(fc::exception& ex){
		elog(ex.to_detail_string());
	}catch(boost::exception& ex){
      elog(boost::diagnostic_information(ex));
   }
	return py_new_none();
}

int get_code_(string& name,string& wast,string& abi,string& code_hash,int& vm_type){
   try{
      chain_controller & db = app().get_plugin<chain_plugin>().chain();
      chain_apis::read_only::get_code_results result;
      result.name = name;
      const auto& d = db.get_database();
      const auto& accnt  = d.get<account_object,by_name>( name );

      if( accnt.code.size() ){
         if (accnt.vm_type == 0){
            result.wast = chain::wasm_to_wast( (const uint8_t*)accnt.code.data(), accnt.code.size() );
         }else{
            result.wast = string((const char*)accnt.code.data(), accnt.code.size());
         }
         result.code_hash = fc::sha256::hash( accnt.code.data(), accnt.code.size() );
      }
      if( accnt.abi.size() > 4 ) {
         eos::types::Abi abi;
         fc::datastream<const char*> ds( accnt.abi.data(), accnt.abi.size() );
         fc::raw::unpack( ds, abi );
         result.abi = std::move(abi);
      }
      vm_type = accnt.vm_type;
      wast = result.wast;
      code_hash = result.code_hash.str();
      abi = fc::json::to_string(fc::variant(result.abi));
      return 0;
   }catch(fc::exception& ex){
      elog(ex.to_detail_string());
   }catch(boost::exception& ex){
      elog(boost::diagnostic_information(ex));
   }
   return -1;
}

int get_table_(string& scope,string& code,string& table,string& result){
	try{
		auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
		chain_apis::read_only::get_table_rows_params params;
		params.json = true;
		params.scope = scope;
		params.code = code;
		params.table = table;
		chain_apis::read_only::get_table_rows_result results = ro_api.get_table_rows(params);
		result = fc::json::to_string(results);
		return 0;
	}catch(fc::exception& ex){
		elog(ex.to_detail_string());
	}catch(boost::exception& ex){
      elog(boost::diagnostic_information(ex));
   }
	return -1;
}


