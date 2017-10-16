#ifndef WALLET_HPP_
#define WALLET_HPP_

#include <string>
#include <vector>
#include <Python.h>

#include <eos/chain/transaction.hpp>

void sign_transaction(eos::chain::SignedTransaction& trx);

int set_timeout_(int secs);

/*
 string sign_transaction(txn,keys,id){
 #    const chain::SignedTransaction& txn, const flat_set<public_key_type>& keys,const chain::chain_id_type& id
 }
 */

PyObject* wallet_create_(std::string& name);
PyObject* wallet_open_(std::string& name);
PyObject* wallet_set_timeout_(int secs);
PyObject* wallet_list_wallets_();
PyObject* wallet_list_keys_();
PyObject* wallet_get_public_keys_();
PyObject* wallet_lock_all_();
PyObject* wallet_lock_(std::string& name);
PyObject* wallet_unlock_(std::string& name, std::string& password);
PyObject* wallet_import_key_(std::string& name, std::string& wif_key);
PyObject* wallet_set_dir_(std::string& path_name);

#endif
