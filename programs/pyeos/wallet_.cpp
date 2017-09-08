#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <iostream>
#include <fc/variant.hpp>
#include <fc/io/json.hpp>
#include <fc/exception/exception.hpp>

#include <fc/log/logger_config.hpp>
#include <boost/thread.hpp>

#include <eos/chain_api_plugin/chain_api_plugin.hpp>
#include <eos/chain/exceptions.hpp>
#include <fc/io/json.hpp>

#include <eos/native_contract/balance_object.hpp>
#include <eos/native_contract/staked_balance_objects.hpp>
#include <eos/chain_plugin/chain_plugin.hpp>
#include <eos/wallet_plugin/wallet_plugin.hpp>
#include <eos/wallet_plugin/wallet_manager.hpp>

#include "pyobject.hpp"
#include <Python.h>


using namespace std;
using namespace eos;
using namespace eos::chain;

wallet_manager& wm(){
   return app().get_plugin<wallet_plugin>().get_wallet_manager();
}

void sign_transaction(SignedTransaction& trx) {
   // TODO better error checking

//    const auto& public_keys = call(wallet_host, wallet_port, wallet_public_keys);
    const auto& public_keys = wm().get_public_keys();
    auto get_arg = fc::mutable_variant_object
         ("transaction", trx)
         ("available_keys", public_keys);

    auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
    auto rw_api = app().get_plugin<chain_plugin>().get_read_write_api();

//    const auto& required_keys = call(host, port, get_required_keys, get_arg);
    eos::chain_apis::read_only::get_required_keys_params params = {fc::variant(trx),public_keys};
    eos::chain_apis::read_only::get_required_keys_result required_keys = ro_api.get_required_keys(params);
    // TODO determine chain id
//    fc::variants sign_args = {fc::variant(trx), required_keys["required_keys"], fc::variant(chain_id_type{})};

//    const auto& signed_trx = call(wallet_host, wallet_port, wallet_sign_trx, sign_args);
//    trx = signed_trx.as<SignedTransaction>();
    auto signed_trx = wm().sign_transaction(trx,required_keys.required_keys,chain_id_type{});
    trx = signed_trx;
}

PyObject* wallet_create_(std::string& name){
    string password = "";
    try{
        password = wm().create(name);
     }catch(fc::exception& ex){
        elog(ex.to_detail_string());
    }catch(...){
    }
    return py_new_string(password);
}

PyObject* wallet_open_(std::string& name){
    try{
        wm().open(name);
    }catch(fc::exception& ex){
        elog(ex.to_detail_string());
        return py_new_bool(false);
    }catch(...){
        return py_new_bool(false);
    }
    return py_new_bool(true);
}

PyObject* wallet_set_dir_(std::string& path_name){
    try{
        wm().set_dir(path_name);
    }catch(fc::exception& ex){
        elog(ex.to_detail_string());
        return py_new_bool(false);
    }catch(...){
        return py_new_bool(false);
    }
    return py_new_bool(true);
}

PyObject* wallet_set_timeout_(int secs){
    try{
        wm().set_timeout(secs);
    }catch(fc::exception& ex){
        elog(ex.to_detail_string());
        return py_new_bool(false);
    }catch(...){
        return py_new_bool(false);
    }
    return py_new_bool(true);
}

/*
string sign_transaction(txn,keys,id){
#    const chain::SignedTransaction& txn, const flat_set<public_key_type>& keys,const chain::chain_id_type& id
}
*/

PyObject* wallet_list_wallets_(){
    PyArray arr;
    try{
            auto list = wm().list_wallets();
            for(auto it=list.begin();it!=list.end();it++){
                arr.append(*it);
            }
    }catch(fc::exception& ex){
        elog(ex.to_detail_string());
    }catch(...){

    }
    return arr.get();
}

PyObject* wallet_list_keys_(){
    PyDict results;
    try{
        map<public_key_type,std::string> keys = wm().list_keys();
        variant v;
        for(auto it=keys.begin();it != keys.end();it++){
//            to_variant(it.first,v);
//            results.add(v.as_string(),key_value.second);
                string key = string(it->first);
                string value = string(it->second);
                results.add(key,value);
        }
    }catch(fc::exception& ex){
        elog(ex.to_detail_string());
    }catch(...){

    }
    return results.get();
}


PyObject* wallet_get_public_keys_(){
    PyArray results;

    try{
        flat_set<public_key_type> keys = wm().get_public_keys();
//        variant v;
        for(auto it=keys.begin();it<keys.end();it++){
//                to_variant(*it,v);
//                results.append(v.as_string());
                results.append((string)*it);
        }
    }catch(fc::exception& ex){
        elog(ex.to_detail_string());
    }catch(...){

    }
    return results.get();
}

PyObject* wallet_lock_all_(){
    try{
        wm().lock_all();
    }catch(fc::exception& ex){
        elog(ex.to_detail_string());
        return py_new_bool(false);
    }
    catch(...){
        return py_new_bool(false);
    }
    return py_new_bool(true);
}

PyObject* wallet_lock_(string& name){
    try{
        wm().lock(name);
    }catch(fc::exception& ex){
        elog(ex.to_detail_string());
        return py_new_bool(false);
    }
    catch(...){
        return py_new_bool(false);
    }
    return py_new_bool(true);
}

PyObject* wallet_unlock_(string& name, string& password){
    try{
        wm().unlock(name,password);
    }catch(fc::exception& ex){
        elog(ex.to_detail_string());
        return py_new_bool(false);
    }
    catch(...){
        return py_new_bool(false);
    }
    return py_new_bool(true);
}

PyObject* wallet_import_key_(string& name,string& wif_key){
    try{
        wm().import_key(name,wif_key);
    }catch(fc::exception& ex){
        elog(ex.to_detail_string());
        return py_new_bool(false);
    }
    catch(...){
        return py_new_bool(false);
    }
    return py_new_bool(true);
}



