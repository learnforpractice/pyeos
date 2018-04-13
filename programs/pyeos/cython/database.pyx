# cython: c_string_type=str, c_string_encoding=utf8
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.map cimport map
from database_object cimport *
from libcpp cimport bool

cdef extern from "":
    ctypedef unsigned long long uint64_t

cdef extern from "database_.hpp" namespace "python::database":
    object database_create(string& path, bool readonly);
    object database_create_account(void* db, string& name);
    object database_get_account(void* db, string& name);
    object database_get_recent_transaction(void* db, string& id);
    object database_get_code(void* db, string& code, int& type);
    object database_flush(void* db, string& id);

#db = database.create(b'config-dir/shared_mem', True);database.get_account(db, b'eosio')

def tobytes(ustr):
    if isinstance(ustr, bytes):
        return ustr
    if isinstance(ustr, str):
        ustr = bytes(ustr, 'utf8')
    return ustr

def create(path, bool readonly=True):
    path = tobytes(path)
    return database_create(path, readonly)

def create_account(uint64_t db, string& name):
    return database_create_account(<void *>db, name)

def get_account(uint64_t db, name):
    return database_get_account(<void*>db, name)

def get_recent_transaction(uint64_t db, string& id):
    return database_get_recent_transaction(<void*>db, id)

def get_code(uint64_t db, string& code, int& type):
    return database_get_code(<void*>db, code, type);

def flush(uint64_t db, string& id):
    return database_flush(<void*>db, id);

