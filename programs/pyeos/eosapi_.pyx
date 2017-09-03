import json
#cdef extern from <stdio.h>:
#    void printf(char *)

class Struct:
    def __init__(self, **entries):
        self.__dict__.update(entries)
    def __str__(self):
        return str(self.__dict__)
    def __repr__(self):
        return str(self.__dict__)

cdef extern from "eosapi.h":
    int get_info_ (char *result,int len)
    int get_block_(int id,char *result,int length)
    int get_account_(char* name,char *result,int length)
    int create_account_( char* creator_,char* newaccount_,char* owner_key_,char* active_key_,char *ts_result,int length)
    int create_key_(char *pub_,int pub_length,char *priv_,int priv_length)
    int get_transaction_(char *id,char* result,int length)
    int transfer_(char *sender_,char* recipient_,int amount,char *result,int length)
    int setcode_(char *account_,char *wast_file,char *abi_file,char *ts_buffer,int length) 
    int exec_func_(char *code_,char *action_,char *json_,char *scope,char *authorization,char *ts_result,int length)


def toobject(bstr):
    bstr = json.loads(bstr.decode('utf8'))
    return Struct(**bstr)


def sayHello():
    print('hello,world')

def get_info():
    cdef char info[1024]
    get_info_(info,1024)
    return toobject(info)

def get_block(int id):
    cdef char info[2048]
    get_block_(id,info,2048)
    return toobject(info)

def get_account(name):
    cdef char info[2048]
    get_account_(name.encode('utf8'),info,2048)
    return toobject(info)

def create_account(creator_,newaccount_,owner_key_,active_key_ ):
    cdef char result[2048]
    cdef int ret
    creator_ = creator_.encode('utf8')
    newaccount_ = newaccount_.encode('utf8')
    owner_key_ = owner_key_.encode('utf8')
    active_key_ = active_key_.encode('utf8')
    ret = create_account_(creator_,newaccount_,owner_key_,active_key_,result,sizeof(result))
    if ret == -1:
        return False
    return True

def create_key():
    cdef char priv[128]
    cdef char pub[128]
    cdef ret
    ret = create_key_(pub,sizeof(pub),priv,sizeof(priv))
    if ret == -1:
        return None
    return (pub,priv)

def unlock():
    raise 'unimplement'

def lock():
    raise 'unimplement'

def do():
    raise 'unimplement'

def get_transaction(id):
    cdef int ret
    cdef char result[2048]
    ret = get_transaction_(id.encode('utf8'),result,sizeof(result))
    if ret == -1:
        return None
    ts = json.loads((<bytes>result).decode('utf8'))
    return Struct(**ts['transaction'])

def transfer(sender_,recipient_,int amount):
    cdef int ret
    cdef char result[2048]
    sender = sender_.encode('utf8')
    recipient = recipient_.encode('utf8')
    ret = transfer_(sender,recipient,amount,result,sizeof(result))
    if ret == -1:
        return None
    return toobject(result)

def setcode(account,wast_file,abi_file):
    cdef int ret
    cdef char ts_buffer[2048]
    account = account.encode('utf8')
    wast_file = wast_file.encode('utf8')
    abi_file = abi_file.encode('utf8')

    ret = setcode_(account,wast_file,abi_file,ts_buffer,sizeof(ts_buffer))
    if ret == -1:
        return None
    return toobject(ts_buffer)

def exec_func(code_,action_,json_,scope_,authorization_):
    cdef int ret
    cdef char ts_result[2048]
    code = code_.encode('utf8')
    action = action_.encode('utf8')
    json = json_.encode('utf8')
    scope = scope_.encode('utf8')
    authorization = authorization_.encode('utf8')
    ret = exec_func_(code,action,json,scope,authorization,ts_result,sizeof(ts_result))
    if ret == -1:
        return None
    return toobject(ts_result)


def test():
    exec(open('test.py').read())

cdef extern char* c_printf(char *s):
    sayHello()




