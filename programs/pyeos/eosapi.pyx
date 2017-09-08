import json
from libcpp.string cimport string
from libcpp.vector cimport vector
from eostypes_ cimport *
#import eostypes_

class JsonStruct:
    def __init__(self, **entries):
        self.__dict__.update(entries)
    def __str__(self):
        return str(self.__dict__)
    def __repr__(self):
        return str(self.__dict__)

cdef extern from "eosapi.h":
    ctypedef int bool
    object get_info_ ()
    object get_block_(char *num_or_id)
    object get_account_(char *name)
    object get_accounts_(char *public_key)
    string create_account_(string creator, string newaccount, string owner, string active, int sign)
    object get_controlled_accounts_(char *account_name);
    void create_key_(string& pub,string& priv)

    string get_transaction_(string id);
    string get_transactions_(string account_name,int skip_seq,int num_seq);

    string push_transaction( SignedTransaction& trx, bool sign )
    int get_transaction_(char *id,char* result,int length)
    int transfer_(char *sender_,char* recipient_,int amount,char *result,int length)
    int setcode_(char *account_,char *wast_file,char *abi_file,char *ts_buffer,int length) 
    int exec_func_(char *code_,char *action_,char *json_,char *scope,char *authorization,char *ts_result,int length)

cdef extern object py_new_bool(int b):
    if b:
        return True
    return False

cdef extern object py_new_none():
    return None

cdef extern object py_new_string(string& s):
    return s

cdef extern object py_new_int(int n):
    return n

cdef extern object py_new_int64(long long n):
    return n

cdef extern object array_create():
    return []

cdef extern void array_append(object arr,object v):
    arr.append(v)

cdef extern void array_append_string(object arr,string& s):
    arr.append(s)

cdef extern void array_append_int(object arr,int n):
    arr.append(n)

cdef extern void array_append_double(object arr,double n):
    arr.append(n)

cdef extern void array_append_uint64(object arr,unsigned long long n):
    arr.append(n)

cdef extern object dict_create():
    return {}

cdef extern void dict_add(object d,object key,object value):
    d[key] = value


def toobject(bstr):
    bstr = json.loads(bstr.decode('utf8'))
    return JsonStruct(**bstr)

def get_info():
    return get_info_()

def get_block(id):
    if type(id) == int:
        id = bytes(id)
    if type(id) == str:
        id = bytes(id,'utf8')
    return get_block_(id)

def get_account(name):
    if type(name) == str:
        name = bytes(name,'utf8')
    return get_account_(name)

def get_accounts(public_key):
    if type(public_key) == str:
        public_key = bytes(public_key,'utf8')
    return get_accounts_(public_key)

def get_controlled_accounts(account_name):
    if type(account_name) == str:
        account_name = bytes(account_name,'utf8')

    return get_controlled_accounts_(account_name);

def create_account(creator,newaccount,owner_key,active_key,sign ):
    if type(creator) == str:
        creator = bytes(creator,'utf8')
    
    if type(newaccount) == str:
        newaccount = bytes(newaccount,'utf8')
    
    if type(owner_key) == str:
        owner_key = bytes(owner_key,'utf8')
    
    if type(active_key) == str:
        active_key = bytes(active_key,'utf8')

    if sign:
        return create_account_(creator,newaccount,owner_key,active_key, 1)
    else:
        return create_account_(creator,newaccount,owner_key,active_key, 0)

def create_key():
    cdef string pub
    cdef string priv
    create_key_(pub,priv)
    return(pub,priv)

def get_transaction(id):
    if type(id) == int:
        id = str(id)
    if type(id) == str:
        id = bytes(id,'utf8')
    return get_transaction_(id)

def get_transactions(account_name,skip_seq,num_seq):
    if type(account_name) == str:
        account_name = bytes(account_name,'utf8')
    return get_transactions_(account_name,skip_seq,num_seq)

def unlock():
    raise 'unimplement'

def lock():
    raise 'unimplement'

def transfer(sender_,recipient_,int amount):
    pass

def setcode(account,wast_file,abi_file):
    pass

def exec_func(code_,action_,json_,scope_,authorization_):
    pass




'''
cdef class PyMessage:
    cdef Message* msg      # hold a C++ instance which we're wrapping
    def __cinit__(self,code,funcName,authorization,data):
#        cdef AccountName code_
#        cdef FuncName funcName_
        cdef Vector[AccountPermission] authorization_
        cdef Bytes data_
        for a in authorization:
            account = bytes(a[0],'utf8')
            permission = bytes(a[1],'utf8')
            authorization_.push_back(AccountPermission(Name(account),Name(permission)))
        for d in bytearray(data,'utf8'):
            data_.push_back(<char>d)
        self.msg = new Message(AccountName(bytes(code,'utf8')),FuncName(bytes(funcName,'utf8')),authorization_,data_)
    def __dealloc__(self):
        del self.msg
'''

