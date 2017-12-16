from cython.operator cimport dereference as deref, preincrement as inc
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.map cimport map

from eostypes_ cimport * 

from typing import Dict, Tuple, List


cdef extern from * :
    ctypedef unsigned long long int64_t

cdef extern from "wallet_.h":
    object wallet_create_(string& name);
    object wallet_open_(string& name);
    object wallet_set_dir_(string& path_name);
    object wallet_set_timeout_(int secs);
    object wallet_list_wallets_();
    object wallet_list_keys_();
    object wallet_get_public_keys_();
    object wallet_lock_all_();
    object wallet_lock_(string& name);
    object wallet_unlock_(string& name, string& password);
    object wallet_import_key_(string& name, string& wif_key);

    object sign_transaction_(void *signed_trx)


def create(name) :
    if type(name) == str:
        name = bytes(name, 'utf8')
    return wallet_create_(name)

def open(name) -> bool:
    if type(name) == str:
        name = bytes(name, 'utf8')
    return wallet_open_(name)

def set_dir(path_name) -> bool:
    if type(path_name) == str:
        path_name = bytes(path_name, 'utf8')
    return wallet_set_dir_(path_name)

def set_timeout(secs) -> bool:
    return wallet_set_timeout_(secs)

def list_wallets() -> List[bytes]:
    return wallet_list_wallets_();

def list_keys() -> Dict[str, str]:
    return wallet_list_keys_();

def get_public_keys():
    return wallet_get_public_keys_();

def lock_all():
    return wallet_lock_all_()

def lock(name) -> bool:
    if type(name) == str:
        name = bytes(name, 'utf8')
    return wallet_lock_(name)

def unlock(name, password) -> bool:
    if type(name) == str:
        name = bytes(name, 'utf8')

    if type(password) == str:
        password = bytes(password, 'utf8')
    
    return wallet_unlock_(name, password)

def import_key(name, wif_key) -> bool:
    if type(name) == str:
        name = bytes(name, 'utf8')
    if type(wif_key) == str:
        wif_key = bytes(wif_key, 'utf8')
    return wallet_import_key_(name, wif_key)

def sign_transaction(signed_trx):
    cdef uint64_t ptr
    ptr = signed_trx()
    return  sign_transaction_(<void*>ptr)

