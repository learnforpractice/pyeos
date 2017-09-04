import json
from libcpp.string cimport string
from libcpp.vector cimport vector

class JsonStruct:
    def __init__(self, **entries):
        self.__dict__.update(entries)
    def __str__(self):
        return str(self.__dict__)
    def __repr__(self):
        return str(self.__dict__)

cdef extern from "eosapi.h":
    object get_info_ ()
    object get_block_(char *num_or_id)
    object get_account_(char *name)
    int create_account_( char* creator_,char* newaccount_,char* owner_key_,char* active_key_,char *ts_result,int length)
    int create_key_(char *pub_,int pub_length,char *priv_,int priv_length)
    int get_transaction_(char *id,char* result,int length)
    int transfer_(char *sender_,char* recipient_,int amount,char *result,int length)
    int setcode_(char *account_,char *wast_file,char *abi_file,char *ts_buffer,int length) 
    int exec_func_(char *code_,char *action_,char *json_,char *scope,char *authorization,char *ts_result,int length)

cdef extern object array_create():
    return []

cdef extern void array_append_string(object arr,string& s):
    arr.append(s)

cdef extern void array_append_int(object arr,int n):
    arr.append(n)

cdef extern void array_append_double(object arr,double n):
    arr.append(n)

cdef extern void array_append_uint64(object arr,unsigned long long n):
    arr.append(n)


def toobject(bstr):
    bstr = json.loads(bstr.decode('utf8'))
    return JsonStruct(**bstr)

def get_info():
    return get_info_()

def get_block(id):
    if type(id) == int:
        id = str(id)
    return get_block_(id.encode('utf8'))

def get_account(name):
    return get_account_(name.encode('utf8'))

def create_account(creator_,newaccount_,owner_key_,active_key_ ):
    pass

def create_key():
    pass

def unlock():
    raise 'unimplement'

def lock():
    raise 'unimplement'

def get_transaction(id):
    pass

def transfer(sender_,recipient_,int amount):
    pass

def setcode(account,wast_file,abi_file):
    pass

def exec_func(code_,action_,json_,scope_,authorization_):
    pass





