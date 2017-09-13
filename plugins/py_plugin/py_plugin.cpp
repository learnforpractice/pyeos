#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <iostream>
#include <fc/variant.hpp>
#include <fc/io/json.hpp>
#include <fc/exception/exception.hpp>

#include <eos/chain/config.hpp>
#include <eos/chain_plugin/chain_plugin.hpp>
#include <eos/account_history_api_plugin/account_history_api_plugin.hpp>

#include <eos/utilities/key_conversion.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/algorithm/string/split.hpp>

#include <Inline/BasicTypes.h>
#include <IR/Module.h>
#include <IR/Validate.h>
#include <WAST/WAST.h>
#include <WASM/WASM.h>
#include <Runtime/Runtime.h>

#include <fc/io/fstream.hpp>


#include <fc/log/logger_config.hpp>
#include <boost/thread.hpp>
#include <eos/py_plugin/py_plugin.hpp>
#include <python.h>



using namespace std;
using namespace eos;
using namespace eos::chain;
using namespace eos::utilities;


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
   catch(Serialization::FatalSerializationException exception)
   {
      std::cerr << "Error serializing WebAssembly binary file:" << std::endl;
      std::cerr << exception.message << std::endl;
      throw;
   }
}


int to_c_string(string s,char* buffer,int length) {
    if (s.size()>length-1) {
        return -1;
    }
    strncpy(buffer,s.data(),length);
    return 0;
}

auto get_info(){
   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
   chain_apis::read_only::get_info_params params;
   return ro_api.get_info(params);
}

int push_transaction( SignedTransaction& trx ,char *ts_result,int length) {
    auto rw_api = app().get_plugin<chain_plugin>().get_read_write_api();
    auto info = get_info();
    trx.expiration = info.head_block_time + 100; //chain.head_block_time() + 100;
    transaction_set_reference_block(trx, info.head_block_id);
    boost::sort( trx.scope );
    try {
        auto params = fc::variant(trx).get_object();
//        auto parms = fc::json::from_string(fc::json::to_string( js )).as<chain_apis::read_write::push_transaction_params>();
//todo
        auto str_result = fc::json::to_pretty_string(rw_api.push_transaction(params));
        return to_c_string(str_result,ts_result,length);
    }
    catch ( const fc::exception& e ) {
        elog((e.to_detail_string()));
    }
    return -1;
}

//   chain_apis::read_write get_read_write_api();
extern "C" int get_info_(char *result,int length) {
   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
   //    auto rw_api = app().get_plugin<chain_plugin>().get_read_write_api();
   chain_apis::read_only::get_info_params params;
   auto info = ro_api.get_info(params);

   auto json_str = fc::json::to_string(info);
   printf("json_str.size():%lu\n",json_str.size());
   if (json_str.size()>length){
      return -1;
   }
   strcpy(result,json_str.data());
   //    ilog(result);
   return 0;
}

extern "C" int get_block_(int id,char *result,int length) {
   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
   char str_id[64];
   sprintf(str_id,"%d",id);
   chain_apis::read_only::get_block_params params = {str_id};
   auto info = ro_api.get_block(params);
   auto json_str = fc::json::to_string(info);
   if (json_str.size()>length){
      return -1;
   }
   strcpy(result,json_str.data());
   return 0;
}

extern "C" int get_account_(char* name,char *result,int length) {
    if (name == NULL || result == NULL){
        return -1;
    }
    try {
        auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
        chain_apis::read_only::get_account_params params = {name};
        auto info = ro_api.get_account(params);
        auto json_str = fc::json::to_string(info);
        if (json_str.size()>length){
            return -1;
        }
        strcpy(result,json_str.data());
        return 0;
    } catch ( const fc::exception& e ) {
        elog((e.to_detail_string()));
    }
    return -1;
}

extern "C" int create_account_( char* creator_,char* newaccount_,char* owner_key_,char* active_key_,char *ts_result,int length) {
    auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();

    Name creator(creator_);
    Name newaccount(newaccount_);
    Name eosaccnt(config::EosContractName);
    Name staked("staked");

    public_key_type owner_key(owner_key_);
    public_key_type active_key(active_key_);

    auto owner_auth   = eos::chain::Authority{1, {{owner_key, 1}}, {}};
    auto active_auth  = eos::chain::Authority{1, {{active_key, 1}}, {}};
    auto recovery_auth = eos::chain::Authority{1, {}, {{{creator, "active"}, 1}}};

    uint64_t deposit = 1;

    SignedTransaction trx;
    trx.scope = sort_names({creator,eosaccnt});

    transaction_emplace_message(trx, config::EosContractName, vector<types::AccountPermission>{{creator,"active"}}, "newaccount",
                                       types::newaccount{creator, newaccount, owner_auth,
                                                         active_auth, recovery_auth, deposit});
    if (creator == "inita")
    {
        fc::optional<fc::ecc::private_key> private_key = eos::utilities::wif_to_key("5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3");
        if (private_key)
        {
            wlog("public key ${k}",("k", private_key->get_public_key()));
            trx.sign(*private_key, eos::chain::chain_id_type{});
        }
    }

    return push_transaction(trx,ts_result,length);
}



extern "C" int create_key_(char *pub_,int pub_length,char *priv_,int priv_length){
   if (pub_ == NULL || priv_ == NULL) {
      return -1;
   }

   auto priv = fc::ecc::private_key::generate();
   auto pub = public_key_type( priv.get_public_key() );

   string str_pub = string(pub);
   if (str_pub.size() > pub_length-1){
      return -1;
   }
   strcpy(pub_,str_pub.data());

   string str_priv = string(key_to_wif(priv.get_secret()));
   if (str_priv.size() > priv_length-1){
      return -1;
   }
   strcpy(priv_,str_priv.data());
   std::cout << "public: " << str_pub <<"\n";
   std::cout << "private: " << str_priv << std::endl;
   return 0;
}

extern "C" int get_transaction_(char *id,char* result,int length){
   if (id==NULL || result==NULL){
      return -1;
   }
   auto ro_api = app().get_plugin<account_history_plugin>().get_read_only_api();

   account_history_apis::read_only::get_transaction_params params = {chain::transaction_id_type(id)};

   //    auto arg= fc::mutable_variant_object( "transaction_id", id);
   string str_ts = fc::json::to_pretty_string(ro_api.get_transaction(params));
//   ilog(str_ts);
   if (str_ts.size()>length-1){
      return -1;
   }
   strcpy(result,str_ts.data());
   return 0;
}


extern "C" int transfer_(char *sender_,char* recipient_,int amount,char *ts_result,int length){
   if (sender_ == NULL || recipient_ == NULL || ts_result == NULL) {
      return -1;
   }
   try {
      auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();

      Name sender(sender_);
      Name recipient(recipient_);
   //   uint64_t amount = fc::variant().as_uint64();
#if 0

      SignedTransaction trx;
      trx.scope = sort_names({sender,recipient});
      transaction_emplace_message(trx, config::EosContractName, vector<types::AccountPermission>{{sender,"active"}}, "transfer",
                         types::transfer{sender, recipient, amount});

      chain_apis::read_only::get_info_params params;
      auto info = ro_api.get_info(params);

      trx.expiration = info.head_block_time + 100; //chain.head_block_time() + 100;
      transaction_set_reference_block(trx, info.head_block_id);
      ilog(trx.id());
      return push_transaction(trx,ts_result,length);
#endif
      string memo;
      SignedTransaction trx;
      trx.scope = sort_names({sender,recipient});
      transaction_emplace_message(trx, config::EosContractName,
                                           vector<types::AccountPermission>{{sender,"active"}},
                                           "transfer", types::transfer{sender, recipient, amount, memo});
      auto info = get_info();
      trx.expiration = info.head_block_time + 100; //chain.head_block_time() + 100;
      transaction_set_reference_block(trx, info.head_block_id);
      ilog(trx.id());
      return push_transaction(trx,ts_result,length);
   }
   catch ( const fc::exception& e ) {
      elog((e.to_detail_string()));
   }
   return -1;
}

extern "C" int setcode_(char *account_,char *wast_file,char *abi_file,char *ts_buffer,int length) {
    Name account(account_);
//    const auto& wast_file = string(file_wast);
    std::string wast;

    FC_ASSERT( fc::exists(wast_file) );
    fc::read_file_contents( wast_file, wast );
    auto wasm = assemble_wast( wast );


    types::setcode handler;
    handler.account = account;
    handler.code.resize(wasm.size());
    memcpy( handler.code.data(), wasm.data(), wasm.size() );

    handler.abi = fc::json::from_file( abi_file ).as<types::Abi>();

    SignedTransaction trx;
    trx.scope = { config::EosContractName, account };
    transaction_emplace_message(trx,  config::EosContractName, vector<types::AccountPermission>{{account,"active"}},
                        "setcode", handler );
    return push_transaction( trx,ts_buffer,length );
//    std::cout << fc::json::to_pretty_string( push_transaction(trx)  ) << std::endl;

}

extern "C" int exec_func_(char *code_,char *action_,char *json_,char *scope,char *authorization,char *ts_result,int length){
    try{
        auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();

        Name code(code_);
        Name action(action_);
//        auto& json   = json_;
//        auto arg= fc::mutable_variant_object( "code", code )("action",action)("args", fc::json::from_string(json));
#if 1
      auto arg= fc::mutable_variant_object
                ("code", code)
                ("action", action)
                ("args", fc::json::from_string(json_));

        auto params = fc::json::from_string(fc::json::to_string(arg)).as<chain_apis::read_only::abi_json_to_bin_params>();
        ilog(params.args.as_string());
//        chain_apis::read_only::abi_json_to_bin_params params = {code,action,fc::variant(json_)};
//        chain_apis::read_only::abi_json_to_bin_params params = {code,action,fc::json::from_string(json_)};

        //        auto result = call( json_to_bin_func, arg);
//        auto result = ro_api.abi_json_to_bin(params);
#endif

#if 0
        SignedTransaction trx;
        trx.messages.resize(1);
        auto& msg = trx.messages.back();
        msg.code = code;
        msg.type = action;
        msg.authorization = fc::json::from_string( authorization ).as<vector<types::AccountPermission>>();
//        msg.data = result.get_object()["binargs"].as<Bytes>();
        msg.data = result.binargs;
        trx.scope = fc::json::from_string(scope).as<vector<Name>>();
#endif
      SignedTransaction trx;
#if 0
      transaction_emplace_serialized_message(trx, code, action,
                                                      vector<types::AccountPermission>{{"currency","active"}},
                                                      fc::variant(result.binargs).as<Bytes>());
#endif

#if 0
        trx.scope = fc::json::from_string(scope).as<vector<Name>>();
        return push_transaction(trx,ts_result,length);
#endif
    }
    catch ( const fc::exception& e ) {
        elog((e.to_detail_string()));
    }
    return -1;
}


namespace eos {

class py_plugin_impl {
   public:
};

py_plugin::py_plugin():my(new py_plugin_impl()){}
py_plugin::~py_plugin(){}

void py_plugin::set_program_options(options_description&, options_description& cfg) {
   cfg.add_options()
         ("option-name", bpo::value<string>()->default_value("default value"),
          "Option Description")
         ;
}

void py_plugin::plugin_initialize(const variables_map& options) {
   if(options.count("option-name")) {
      // Handle the option
   }
}

extern "C" void c_printf(const char *s);
extern "C" void PyInit_eosapi();

void py_thread() {
   Py_Initialize();
   PyRun_SimpleString("import readline");
   PyInit_eosapi();
   PyRun_SimpleString("import eosapi;import sys;sys.path.append('./eosd')");
   PyRun_SimpleString("from initeos import *");

   ilog("++++++++++++++py_plugin::plugin_startup");
   c_printf("hello,world");
   //    get_info();
   PyRun_InteractiveLoop(stdin, "<stdin>");
   Py_Finalize();
}

void py_plugin::plugin_startup() {
   boost::thread t{py_thread};
}

void py_plugin::plugin_shutdown() {
   ilog("py_plugin::plugin_shutdown()");
   Py_Finalize();
}

}
