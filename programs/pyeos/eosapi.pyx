import json
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.map cimport map
from eostypes_ cimport *
#import eostypes_
from typing import Dict, Tuple, List

cdef extern from "<fc/log/logger.hpp>":
    void ilog(char *log)


class JsonStruct:
    def __init__(self, **entries):
        self.__dict__.update(entries)
    def __str__(self):
        return str(self.__dict__)
    def __repr__(self):
        return str(self.__dict__)

cdef extern from "eosapi.h":
    ctypedef int bool
    void quit_app_()
    
    object get_info_ ()
    object get_block_(char *num_or_id)
    object get_account_(char *name)
    object get_accounts_(char *public_key)
    int create_account_(string creator, string newaccount, string owner, string active, int sign,string& result)
    object get_controlled_accounts_(char *account_name);
    void create_key_(string& pub,string& priv)

    int get_transaction_(string& id,string& result);
    int get_transactions_(string& account_name,int skip_seq,int num_seq,string& result);

    int get_transactions_(string& account_name,int skip_seq,int num_seq,string& result);
    
    int transfer_(string& sender,string& recipient,int amount,string memo,bool sign,string& result);
    int push_message_(string& contract,string& action,string& args,vector[string] scopes,map[string,string]& permissions,bool sign,string& ret);
    int set_contract_(string& account,string& wastPath,string& abiPath,int vmtype,bool sign,string& result);
    int get_code_(string& name,string& wast,string& abi,string& code_hash);
    int get_table_(string& scope,string& code,string& table,string& result);

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
    bstr = json.loads(bstr)
    return JsonStruct(**bstr)

def tobytes(ustr:str):
    if type(ustr) == str:
        ustr = bytes(ustr,'utf8')
    return ustr

def get_info()->str:
    return get_info_()

def get_block(id:str)->str:
    if type(id) == int:
        id = bytes(id)
    if type(id) == str:
        id = bytes(id,'utf8')
    return get_block_(id)

def get_account(name:str)->str:
    if type(name) == str:
        name = bytes(name,'utf8')
    return get_account_(name)

def get_accounts(public_key:str)->List[str]:
    if type(public_key) == str:
        public_key = bytes(public_key,'utf8')
    return get_accounts_(public_key)

def get_controlled_accounts(account_name:str)->List[str]:
    if type(account_name) == str:
        account_name = bytes(account_name,'utf8')

    return get_controlled_accounts_(account_name);

def create_account(creator:str,newaccount:str,owner_key:str,active_key:str,sign)->str:
    cdef string result
    if type(creator) == str:
        creator = bytes(creator,'utf8')
    
    if type(newaccount) == str:
        newaccount = bytes(newaccount,'utf8')
    
    if type(owner_key) == str:
        owner_key = bytes(owner_key,'utf8')
    
    if type(active_key) == str:
        active_key = bytes(active_key,'utf8')
    if sign:
        sign = 1
    else:
        sign = 0

    if 0 == create_account_(creator,newaccount,owner_key,active_key, sign,result):
        return result
    return None

def create_key()->Tuple[bytes]:
    cdef string pub
    cdef string priv
    create_key_(pub,priv)
    return(pub,priv)

def get_transaction(id:str)->str:
    cdef string result
    if type(id) == int:
        id = str(id)
    id = tobytes(id)
    if 0 == get_transaction_(id,result):
        return result
    return None

def get_transactions(account_name:str,skip_seq:int,num_seq:int)->str:
    cdef string result
    account_name = tobytes(account_name)
    if 0 == get_transactions_(account_name,skip_seq,num_seq,result):
        return result
    return None

def transfer(sender:str,recipient:str,int amount,memo:str,sign)->str:
    cdef string result
    sender = tobytes(sender)
    recipient = tobytes(recipient)
    memo = tobytes(memo)
    if sign:
        sign = 1
    else:
        sign = 0
    if 0 == transfer_(sender,recipient,amount,memo,sign,result):
        return result
    return None

def push_message(contract:str,action:str,args:str,scopes:List[str],permissions:Dict,sign):
    cdef string ret
    cdef vector[string] scopes_;
    cdef map[string,string] permissions_;
    contract = tobytes(contract)
    action = tobytes(action)
    args = tobytes(args)
    
    for scope in scopes:
        scopes_.push_back(tobytes(scope))
    for per in permissions:
        key = permissions[per]
        per = tobytes(per)
        key = tobytes(key)
        permissions_[per] = key

    if sign:
        sign = 1
    else:
        sign = 0

    if 0 == push_message_(contract,action,args,scopes_,permissions_,sign,ret):
        return ret
    return None

def set_contract(account:str,wast_file:str,abi_file:str,vmtype:int,sign)->str:
    cdef string result
    ilog("set_contract.....");
    account = tobytes(account)
    wast_file = tobytes(wast_file)
    abi_file = tobytes(abi_file)
    if sign:
        sign = 1
    else:
        sign = 0

    if 0 == set_contract_(account,wast_file,abi_file,vmtype,sign,result):
        return result
    return None

def get_contract(name:str):
    cdef string wast
    cdef string abi
    cdef string code_hash
    name = tobytes(name)
    if 0 == get_code_(name,wast,abi,code_hash):
        return [wast,abi,code_hash]
    return []

def get_table(scope,code,table):
    cdef string result
    scope = tobytes(scope)
    code = tobytes(code)
    table = tobytes(table)

    if 0 == get_table_(scope,code,table,result):
        return result
    return None

def exec_func(code_:str,action_:str,json_:str,scope_:str,authorization_:str)->str:
    pass

def quit_app():
    quit_app_();

import signal
import sys
import time
app_quit = False
def signal_handler(signal, frame):
    global app_quit
    if app_quit:
        sys.exit(0)
        return
    print('shutting down... you should wait for database closed successfully,\nthen press Ctrl+C again to exit application!sorry about that.')
    quit_app()
    app_quit = True
    
#    while not app_isshutdown_():
#        time.sleep(0.2) # wait for app shutdown
#    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)




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

