#include <Python.h>

#ifndef __HELLO_H
#define __HELLO_H
#ifdef __cplusplus

extern "C" {
#endif
    PyObject* get_info_();
	PyObject *get_block_(char *num_or_id);
    PyObject* get_account_(char *name);
    int create_account_( char* creator_,char* newaccount_,char* owner_key_,char* active_key_,char *ts_result,int length);
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
#include <python.h>


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

chain_controller& db();
string to_bitset(uint64_t val);
wallet_manager& get_wm();



