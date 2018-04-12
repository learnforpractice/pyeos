#ifndef WALLET_HPP_
#define WALLET_HPP_

#include <Python.h>
#include <string>
#include <vector>

#include <eosio/chain/transaction.hpp>

void sign_transaction(eosio::chain::signed_transaction &trx);

int set_timeout_(int secs);

/*
 string sign_transaction(txn,keys,id){
 #    const chain::signed_transaction& txn, const flat_set<public_key_type>&
 keys,const chain::chain_id_type& id
 }
 */

PyObject* wallet_create_(std::string &name);
PyObject* wallet_open_(std::string &name);
PyObject* wallet_save_(std::string& name);

PyObject* wallet_set_timeout_(int secs);
PyObject* wallet_list_wallets_();
PyObject* wallet_list_keys_();
PyObject* wallet_get_public_keys_();
PyObject* wallet_lock_all_();
PyObject* wallet_lock_(std::string &name);
PyObject* wallet_unlock_(std::string &name, std::string &password);
PyObject* wallet_import_key_(std::string &name, std::string &wif_key, bool save);
PyObject* wallet_set_dir_(std::string &path_name);

PyObject* sign_transaction_(void *signed_trx);


#endif
