from cython.operator cimport dereference as deref, preincrement as inc
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.map cimport map

from eostypes_ cimport *

cdef extern from *:
    ctypedef unsigned long long int64_t

cdef extern from "<eos/wallet_plugin/wallet_manager.hpp>" namespace "eos::wallet":

    cdef cppclass wallet_manager:
        void set_dir(const path& p)
        void set_timeout(int64_t secs)
#        chain::SignedTransaction sign_transaction(const chain::SignedTransaction& txn, const flat_set<public_key_type>& keys,\
#                                                 const chain::chain_id_type& id);
        string create(const string& name);
 
        void open(const string& name);
 
        vector[string] list_wallets();
 
        map[public_key_type,string] list_keys();
 
        flat_set[public_key_type] get_public_keys();
 
        void lock_all();
 
        void lock(const string& name);
 
        void unlock(const string& name, const string& password);
 
        void import_key(const string& name, const string& wif_key);

cdef extern from "eosapi.h":
    wallet_manager& get_wm()

def create(name):
    name = bytes(name,'utf8')
    return get_wm().create(name)

def set_dir(path_name):
    path_name = bytes(path_name,'utf8')
    set_dir(path_name)
#        void set_dir(const boost::filesystem::path& p) { dir = p; }

def set_timeout(secs):
    get_wm().set_timeout(secs)

def sign_transaction(txn,keys,id):
    pass

def sign_transaction(txn,keys,id):
#    const chain::SignedTransaction& txn, const flat_set<public_key_type>& keys,const chain::chain_id_type& id
    pass

def open(name):
    name = bytes(name,'utf8')
    return get_wm().open(name)

def list_wallets():
    return get_wm().list_wallets();

def list_keys():
#    cdef map[public_key_type,string].value_type *key_value
    cdef map[public_key_type,string] keys = get_wm().list_keys()
    cdef map[public_key_type,string].iterator it = keys.begin()
    cdef variant v
    print(keys.size())
    result = {}
    while it != keys.end():
        key_value = deref(it)
        to_variant(key_value.first,v)
        result[v.as_string()] = key_value.second
#        print(key_value.second)
        inc(it)
    return result

#        flat_set<public_key_type> get_public_keys();

def get_public_keys():
    cdef flat_set[public_key_type] keys = get_wm().get_public_keys()
    cdef flat_set[public_key_type].iterator it = keys.begin()
    cdef variant v
    result = []
    while it != keys.end():
        value = deref(it)
        inc(it)
        to_variant(value,v)
        result.append(v.as_string())
    return result

def lock_all():
    get_wm().lock_all()

def lock(name):
    name = bytes(name,'utf8')
    get_wm().lock(name)

def unlock(name, password):
    name = bytes(name,'utf8')
    password = bytes(password,'utf8')
    get_wm().unlock(name,password)

def import_key(name,wif_key):
    name = bytes(name,'utf8')
    wif_key = bytes(name,'utf8')
    get_wm().import_key(name,wif_key)

