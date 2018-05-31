# cython: c_string_type=str, c_string_encoding=ascii

from cython.operator cimport dereference as deref, preincrement as inc
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.map cimport map
from libcpp cimport bool

from eostypes_ cimport * 

from typing import Dict, Tuple, List


cdef extern from * :
    ctypedef unsigned long long int64_t

cdef extern from "wallet_.h":
    object wallet_create_(string& name);
    object wallet_save_(string& name);

    object wallet_open_(string& name);
    object wallet_set_dir_(string& path_name);
    object wallet_set_timeout_(int secs);
    object wallet_list_wallets_();
    object wallet_list_keys_(const string& name, const string& pw);
    object wallet_get_public_keys_();
    object wallet_lock_all_();
    object wallet_lock_(string& name);
    object wallet_unlock_(string& name, string& password);
    object wallet_import_key_(string& name, string& wif_key, bool save);

    object sign_transaction_(void *signed_trx)


def create(string& name) :
    return wallet_create_(name)

def save(string& name) :
    return wallet_save_(name)

def open(string& name) -> bool:
    return wallet_open_(name)

def set_dir(string& path_name) -> bool:
    return wallet_set_dir_(path_name)

def set_timeout(secs) -> bool:
    return wallet_set_timeout_(secs)

def list_wallets() -> List[bytes]:
    return wallet_list_wallets_();

def list_keys(string& name, string& psw) -> Dict[str, str]:
    return wallet_list_keys_(name, psw);

def get_public_keys():
    return wallet_get_public_keys_();

def lock_all():
    return wallet_lock_all_()

def lock(string& name) -> bool:
    return wallet_lock_(name)

def unlock(string& name, string& password) -> bool:
    return wallet_unlock_(name, password)

def import_key(string& name, string& wif_key, save=True) -> bool:
    return wallet_import_key_(name, wif_key, save)

def sign_transaction(signed_trx):
    cdef uint64_t ptr
    ptr = signed_trx()
    return  sign_transaction_(<void*>ptr)

