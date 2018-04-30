#include <Python.h>

#ifndef __HELLO_H
#define __HELLO_H
#ifdef __cplusplus

extern "C" {
#endif
int create_key_(char* pub_, int pub_length, char* priv_, int priv_length);
int get_transaction_(char* id, char* result, int length);
int transfer_(char* sender_, char* recipient_, int amount, char* result,
              int length);
int setcode_(char* account_, char* wast_file, char* abi_file, char* ts_buffer,
             int length);
int exec_func_(char* code_, char* action_, char* json_, char* scope,
               char* authorization, char* ts_result, int length);
#ifdef __cplusplus
}
#endif
#endif

#include <boost/asio.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/json.hpp>
#include <fc/variant.hpp>
#include <iostream>
#include <string>
#include <vector>

#include <eosio/account_history_api_plugin/account_history_api_plugin.hpp>
#include <eosio/chain/config.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <eosio/utilities/key_conversion.hpp>

#include <IR/Module.h>
#include <IR/Validate.h>
#include <Inline/BasicTypes.h>
#include <Runtime/Runtime.h>
#include <WASM/WASM.h>
#include <WAST/WAST.h>

#include <fc/io/fstream.hpp>

#include <Python.h>
#include <boost/thread.hpp>
#include <eos/py_plugin/py_plugin.hpp>
#include <fc/log/logger_config.hpp>

#include <eosio/chain/exceptions.hpp>
#include <eosio/chain_api_plugin/chain_api_plugin.hpp>
#include <eosio/producer_plugin/producer_plugin.hpp>
#include <fc/io/json.hpp>

#include <eosio/chain/wast_to_wasm.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/wallet_plugin/wallet_manager.hpp>
#include <eosio/wallet_plugin/wallet_plugin.hpp>

using namespace std;
using namespace eosio;
using namespace eosio::chain;
using namespace eosio::utilities;
using namespace eosio::wallet;
using namespace eosio::chain_apis;

void quit_app_();
bool app_isshutdown_();
uint32_t now2_();

chain_controller& db();
string to_bitset(uint64_t val);
wallet_manager& get_wm();
PyObject* push_transaction(signed_transaction& trx, bool sign);
PyObject* push_transaction2_(void* signed_trx, bool sign);

#define INT_65 65

int produce_block_();

PyObject* get_info_();
PyObject* get_block_(char* num_or_id);
PyObject* get_account_(const char* name);

PyObject* get_currency_balance_(string& _code, string& _account, string& _symbol);

PyObject* get_accounts_(char* public_key);
PyObject* get_controlled_accounts_(char* account_name);

PyObject* create_account_(string creator, string newaccount, string owner,
                          string active, int sign);

PyObject* set_evm_contract_(string& eth_address, string& sol_bin, bool sign);

PyObject* create_key_();
PyObject* get_public_key_(string& wif_key);

int get_transaction_(string& id, string& result);
int get_transactions_(string& account_name, int skip_seq, int num_seq,
                      string& result);
PyObject* transfer_(string& sender, string& recipient, int amount, string memo,
                    bool sign);
PyObject* push_message_(string& contract, string& action, string& args,
                        map<string, string>& permissions,
                        bool sign, bool rawargs);
PyObject* set_contract_(string& account, string& wastPath, string& abiPath,
                        int vmtype, bool sign);
int get_code_(string& name, string& wast, string& abi, string& code_hash,
              int& vm_type);
int get_table_(string& scope, string& code, string& table, string& result);

PyObject* traceback_();


uint64_t string_to_uint64_(string str);
string uint64_to_string_(uint64_t n);
string convert_to_eth_address(string& name);
string convert_from_eth_address(string& eth_address);

PyObject* push_messages_(vector<string>& contracts, vector<string>& functions, vector<string>& args,
      vector<map<string, string>>& permissions, bool sign, bool rawargs);

PyObject* push_transactions_(vector<string>& contracts, vector<string>& functions, vector<string>& args,
      vector<map<string, string>>& permissions, bool sign, bool rawargs);

PyObject* push_transactions2_(vector<vector<chain::action>>& vv, bool sign, uint64_t skip_flag, bool async);


PyObject* push_messages_ex_(string& contracts, vector<string>& functions, vector<string>& args, map<string, string>& permissions,
                         bool sign, bool rawargs);


int compile_and_save_to_buffer_(const char* src_name, const char *src_buffer, size_t src_size, char* buffer, size_t size);

void wast2wasm_( string& wast ,string& result);

bool is_replay_();

void pack_bytes_(string& in, string& out);
void unpack_bytes_(string& in, string& out);

void fc_pack_setabi_(string& abiPath, uint64_t account, string& out);
