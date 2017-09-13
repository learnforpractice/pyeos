#include <Python.h>

#ifndef __HELLO_H
#define __HELLO_H
#ifdef __cplusplus

extern "C" {
#endif
	int create_key_(char *pub_,int pub_length,char *priv_,int priv_length);
    int get_transaction_(char *id,char* result,int length);
    int transfer_(char *sender_,char* recipient_,int amount,char *result,int length);
    int setcode_(char *account_,char *wast_file,char *abi_file,char *ts_buffer,int length);
    int exec_func_(char *code_,char *action_,char *json_,char *scope,char *authorization,char *ts_result,int length);
#ifdef __cplusplus
}
#endif
#endif

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
#include <Python.h>


#include <eos/chain_api_plugin/chain_api_plugin.hpp>
#include <eos/chain/exceptions.hpp>
#include <fc/io/json.hpp>

#include <eos/native_contract/balance_object.hpp>
#include <eos/native_contract/staked_balance_objects.hpp>
#include <eos/chain_plugin/chain_plugin.hpp>
#include <eos/wallet_plugin/wallet_plugin.hpp>
#include <eos/wallet_plugin/wallet_manager.hpp>


using namespace std;
using namespace eos;
using namespace eos::chain;
using namespace eos::utilities;
using namespace eos::wallet;
using namespace eos::chain_apis;

chain_controller& db();
string to_bitset(uint64_t val);
wallet_manager& get_wm();
string push_transaction( SignedTransaction& trx, bool sign );

#define INT_65 65

PyObject* get_info_();
PyObject* get_block_(char *num_or_id);
PyObject* get_account_(char *name);
PyObject* get_accounts_(char *public_key);
PyObject* get_controlled_accounts_(char *account_name);

string create_account_(string creator, string newaccount, string owner, string active, int sign);
void create_key_(string& pub,string& priv);

int get_transaction_(string& id,string& result);
int get_transactions_(string& account_name,int skip_seq,int num_seq,string& result);
int transfer_(string& sender,string& recipient,int amount,string memo,bool sign,string& result);
int push_message_(string& contract,string& action,string& args,vector<string> scopes,map<string,string>& permissions,bool sign,string& ret);
int set_contract_(string& account,string& wastPath,string& abiPath,int vmtype,bool sign,string& result);
int get_code_(string& name,string& wast,string& abi,string& code_hash);
int get_table_(string& scope,string& code,string& table,string& result);



