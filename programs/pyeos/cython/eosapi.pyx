# cython: c_string_type=str, c_string_encoding=ascii

import os
import sys
import signal
import atexit

import time
import json
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.map cimport map
from eostypes_ cimport * 

import wallet

# import eostypes_
from typing import Dict, Tuple, List

cdef extern from "<fc/log/logger.hpp>":
    void ilog(char* log)

cdef extern from "<fc/crypto/xxhash.h>":
    uint64_t XXH64(const char *data, size_t length, uint64_t seed);

cdef extern from "<eosio/chain/wast_to_wasm.hpp>":
    ctypedef unsigned char uint8_t
    vector[uint8_t] wast_to_wasm( string& wast )

cdef extern from "<eosio/chain/contracts/types.hpp>" namespace "eosio::chain::contracts":
    cdef cppclass setcode:
        uint64_t                     account;
        unsigned char                            vmtype;
        unsigned char                            vmversion;
        vector[char]                            code;

cdef extern from "<fc/io/raw.hpp>" namespace "fc::raw":
    cdef vector[char] pack[T](T& code)

cdef extern from "eosapi_.hpp":
    ctypedef int bool

    uint64_t string_to_uint64_(string str);
    string uint64_to_string_(uint64_t n);
    string convert_to_eth_address(string& name)
    string convert_from_eth_address(string& eth_address)

    void quit_app_()
    uint32_t now2_()
    int produce_block_();
    object get_info_ ()
    object get_block_(char* num_or_id)
    object get_account_(char* name)
    object get_accounts_(char* public_key)
    object create_account_(string creator, string newaccount, string owner, string active, int sign)
    object get_controlled_accounts_(char* account_name);
    object create_key_()
    object get_public_key_(string& wif_key)

    object push_transaction2_(void* signed_trx, bool sign)

    int get_transaction_(string& id, string& result);
    int get_transactions_(string& account_name, int skip_seq, int num_seq, string& result);
    
    object transfer_(string& sender, string& recipient, int amount, string memo, bool sign);
    object push_message_(string& contract, string& action, string& args, map[string, string]& permissions, bool sign, bool rawargs)
    object set_contract_(string& account, string& wastPath, string& abiPath, int vmtype, bool sign);
    object set_evm_contract_(string& eth_address, string& sol_bin, bool sign);

    int get_code_(string& name, string& wast, string& abi, string& code_hash, int & vm_type);
    int get_table_(string& scope, string& code, string& table, string& result);

    int setcode_(char* account_, char* wast_file, char* abi_file, char* ts_buffer, int length) 
    int exec_func_(char* code_, char* action_, char* json_, char* scope, char* authorization, char* ts_result, int length)

    object get_currency_balance_(string& _code, string& _account, string& _symbol)

    object traceback_()

    object push_messages_(vector[string]& contract, vector[string]& functions, vector[string]& args, vector[map[string, string]]& permissions,bool sign, bool rawargs)

    object push_messages_ex_(string& contract, vector[string]& functions, vector[string]& args, map[string, string]& permissions,bool sign, bool rawargs)

    object push_transactions_(vector[string]& contracts, vector[string]& functions, vector[string]& args, vector[map[string, string]]& permissions, bool sign, bool rawargs);

    int compile_and_save_to_buffer_(const char* src_name, const char *src_buffer, size_t src_size, char* buffer, size_t size);

    void wast2wasm_( string& wast ,string& result)
    bool is_replay_();

    void pack_bytes_(string& _in, string& out);
    void unpack_bytes_(string& _in, string& out);

    cdef cppclass permission_level:
        permission_level()
        uint64_t    actor;
        uint64_t permission;

    cdef cppclass action:
        action()
        uint64_t                    account;
        uint64_t                    name;
        vector[permission_level]    authorization;
        vector[char]                data;

    object push_transactions2_(vector[vector[action]]& actions, bool sign, uint64_t skip_flag, bool _async)
    void memcpy(char* dst, char* src, size_t len)
#    void fc_pack_setcode(setcode _setcode, vector<char>& out)

    void fc_pack_setabi_(string& abiPath, uint64_t account, string& out)

VM_TYPE_WASM = 0
VM_TYPE_PY = 1
VM_TYPE_MP = 2

py_vm_type = VM_TYPE_MP

class JsonStruct(object):
    def __init__(self, js):
        if isinstance(js, bytes):
            js = js.decode('utf8')
            js = json.loads(js)
        if isinstance(js, str):
            js = json.loads(js)
        for key in js:
            value = js[key]
            if isinstance(value, dict):
                self.__dict__[key] = JsonStruct(value)
            elif isinstance(value, list):
                for i in range(len(value)):
                    v = value[i]
                    if isinstance(v, dict):
                        value[i] = JsonStruct(v)
                self.__dict__[key] = value
            else:
                self.__dict__[key] = value
    def __str__(self):
        return str(self.__dict__)
#        return json.dumps(self, default=lambda x: x.__dict__,sort_keys=False,indent=4, separators=(',', ': '))
    def __repr__(self):
        return json.dumps(self, default=lambda x: x.__dict__, sort_keys=False, indent=4, separators=(',', ': '))

def s2n(string& name):
    ret = string_to_uint64_(name)
    if ret == 0:
        raise Exception('bad name {0}'.format(name))
    return ret

def n2s(n):
    return uint64_to_string_(n)

def N(name):
    return s2n(name)

def eos_name_to_eth_address(string name):
    return convert_to_eth_address(name)

def eth_address_to_eos_name(string addr):
    return convert_from_eth_address(addr)

def toobject(bstr):
    return JsonStruct(bstr)

def tobytes(ustr):
    if isinstance(ustr, bytes):
        return ustr
    if isinstance(ustr, str):
        ustr = bytes(ustr, 'utf8')
    return ustr

def now():
    return now2_()

def produce_block():
    ret = produce_block_()
    time.sleep(0.5)
    return ret

def get_info():
    info = get_info_()
    return JsonStruct(info)

def get_block(id):
    if isinstance(id, int):
        id = str(id)
        id = bytes(id, 'utf8')
    if isinstance(id, str):
        id = bytes(id, 'utf8')
    info = get_block_(id)
    return JsonStruct(info)

def get_account(name):
    if isinstance(name, str):
        name = bytes(name, 'utf8')
    result = get_account_(name)
    if result:
        return JsonStruct(result)
    return None

def get_accounts(public_key):
    if isinstance(public_key, str):
        public_key = bytes(public_key, 'utf8')
    return get_accounts_(public_key)

def get_currency_balance(string _code, string _account, string _symbol = ''):
    return get_currency_balance_(_code, _account, _symbol)

'''
def get_controlled_accounts(account_name) -> List[str]:
    if isinstance(account_name, str):
        account_name = bytes(account_name, 'utf8')

    return get_controlled_accounts_(account_name);
'''

def create_account(creator, newaccount, owner_key, active_key, sign=True):
    if isinstance(creator, str):
        creator = bytes(creator, 'utf8')
    
    if isinstance(newaccount, str):
        newaccount = bytes(newaccount, 'utf8')
    
    if isinstance(owner_key, str):
        owner_key = bytes(owner_key, 'utf8')
    
    if isinstance(active_key, str):
        active_key = bytes(active_key, 'utf8')
    if sign:
        sign = 1
    else:
        sign = 0

    result = create_account_(creator, newaccount, owner_key, active_key, sign)
    if result:
        return JsonStruct(result)
    return None

def create_key():
    cdef string pub
    cdef string priv
    key = create_key_()
    return JsonStruct(key)

def get_public_key(priv_key):
    return get_public_key_(priv_key)

def get_transaction(id):
    cdef string result
    if isinstance(id, int):
        id = str(id)
    if 0 == get_transaction_(id, result):
        return JsonStruct(result)
    return None

def get_transactions(account_name, skip_seq: int, num_seq: int):
    cdef string result
    if 0 == get_transactions_(account_name, skip_seq, num_seq, result):
        return result
    return None

def transfer(sender, recipient, int amount, memo, sign=True):
    memo = tobytes(memo)
    if sign:
        sign = 1
    else:
        sign = 0
    result = transfer_(sender, recipient, amount, memo, sign)
    if result:
        return JsonStruct(result)
    return None

def push_message(string& contract, string& action, args, permissions: Dict, sign=True, rawargs=False):
    cdef map[string, string] permissions_;
    cdef int sign_
    cdef int rawargs_
    if not rawargs:
        args = json.dumps(args)

    for per in permissions:
        key = permissions[per]
        permissions_[per] = key

    if sign:
        sign_ = 1
    else:
        sign_ = 0
    if rawargs:
        rawargs_ = 1
    else:
        rawargs_ = 0
#    print(contract_,action_,args_,scopes_,permissions_,sign_)
    result = push_message_(contract, action, args, permissions_, sign_, rawargs_)
    if result:
        return JsonStruct(result)
    return None

def push_evm_message(eth_address, args, permissions: Dict, sign=True, rawargs=False):
    cdef string contract_
    cdef string action_
    cdef string args_
    cdef map[string, string] permissions_;
    cdef int sign_
    cdef int rawargs_
    
    contract_ = convert_from_eth_address(eth_address)
    print('===eth_address:', eth_address)
    print('===contract_:', contract_)

    if not rawargs:
        if not isinstance(args, str):
            args = json.dumps(args)
    args_ = tobytes(args)
    
    for per in permissions:
        key = permissions[per]
        per = convert_from_eth_address(per)
        permissions_[per] = key

    if sign:
        sign_ = 1
    else:
        sign_ = 0
    if rawargs:
        rawargs_ = 1
    else:
        rawargs_ = 0
    result = push_message_(contract_, '', args_, permissions_, sign_, rawargs_)
    if result:
        return JsonStruct(result)
    return None


def set_contract(string& account, string& wast_file, string& abi_file, vmtype=1, sign=True):
    if not os.path.exists(wast_file):
        return False
    if sign:
        sign = 1
    else:
        sign = 0

    result = set_contract_(account, wast_file, abi_file, vmtype, sign)
    
    if result:
        return JsonStruct(result)
    return None

def set_evm_contract(eth_address, sol_bin, sign=True):
    ilog("set_evm_contract.....");
    if sign:
        sign = 1
    else:
        sign = 0
    if sol_bin[0:2] == '0x':
        sol_bin = sol_bin[2:]
    result = set_evm_contract_(eth_address, sol_bin, sign);

    if result:
        return JsonStruct(result)
    return None

def get_code(name):
    cdef string wast
    cdef string abi
    cdef string code_hash
    cdef int vm_type

    vm_type = 0
    if 0 == get_code_(name, wast, abi, code_hash, vm_type):
        return [<bytes>wast, <bytes>abi, code_hash, vm_type]
    return []

def get_table(string& scope, string& code, string& table):
    cdef string result

    if 0 == get_table_(scope, code, table, result):
        return JsonStruct(result)
    return None

def exec_func(code_:str, action_:str, json_:str, scope_:str, authorization_:str):
    pass

from importlib.abc import Loader, MetaPathFinder
from importlib.util import spec_from_file_location

class CodeLoader(Loader):
    def __init__(self, code):
        self.code = code
    def create_module(self, spec):
        return None  # use default module creation semantics
    def exec_module(self, module):
        exec(self.code, vars(module))

class MetaFinder(MetaPathFinder):
    def find_spec(self, contract_name, path, target=None):
        print(contract_name, path, target)
        code = get_code(contract_name)
        if not code:
            return None
        if code[-1] != 1:
            return None
        return spec_from_file_location(contract_name, None, loader=CodeLoader(code[0]), submodule_search_locations=None)

def install():
    sys.meta_path.insert(0, MetaFinder())

cdef extern set_args(int argc, char* argv[]):
    import sys
    argv_ = []
    for i in range(argc):
        argv_.append(argv[i])
    sys.argv = argv_

class Producer(object):
    def __init__(self):
        pass
    
    def produce_block(self):
        ret = produce_block_()
        time.sleep(0.5)

    def __call__(self):
        self.produce_block()

    def __enter__(self):
        pass
    
    def __exit__(self, type, value, traceback):
        self.produce_block()

def push_transaction2(signed_trx,sign=True):
    cdef uint64_t ptr
    ptr = signed_trx()
    return push_transaction2_(<void*>ptr,sign)

def traceback():
    return traceback_()

def quit_app():
    quit_app_()

exit_by_signal_handler = False

def signal_handler(signal, frame):
    exit()

cdef extern void py_exit() with gil:
    exit()

def register_signal_handler():
    signal.signal(signal.SIGINT, signal_handler)

def on_python_exit():
    quit_app_()

atexit.register(on_python_exit)


def push_messages(vector[string]& contracts, vector[string]& functions, args, permissions,bool sign, bool rawargs):
    cdef vector[map[string, string]] _permissions
    cdef map[string, string] __permissions;
    cdef vector[string] _args
    
    for per in permissions:#list
        __permissions = map[string, string]()
        for _key in per: #dict
            value = per[_key]
            __permissions[_key] = value
        _permissions.push_back(__permissions)

    for arg in args:
        if isinstance(arg, dict):
            arg = json.dumps(arg)
        _args.push_back(arg)

    ret = push_messages_(contracts, functions, _args, _permissions,sign, rawargs)
    return (ret)

def push_messages_ex(string& contract, vector[string]& functions, args, permissions,bool sign, bool rawargs):
    cdef map[string, string] _permissions
    cdef vector[string] _args
    
    for per in permissions:
        key = permissions[per]
        _permissions[per] = key

    for arg in args:
        if isinstance(arg, dict):
            arg = json.dumps(arg)
        _args.push_back(arg)

    ret = push_messages_ex_(contract, functions, _args, _permissions,sign, rawargs)
    return (ret)


def push_transactions(vector[string]& contracts, vector[string]& functions, args, permissions,bool sign, bool rawargs):
    cdef vector[map[string, string]] _permissions
    cdef map[string, string] __permissions;
    cdef vector[string] _args
    
    for per in permissions:#list
        __permissions = map[string, string]()
        for _key in per: #dict
            value = per[_key]
            __permissions[_key] = value
        _permissions.push_back(__permissions)

    for arg in args:
        if isinstance(arg, dict):
            arg = json.dumps(arg)
        _args.push_back(arg)

    ret = push_transactions_(contracts, functions, _args, _permissions,sign, rawargs)
    return (ret)

def push_transactions2(actions, bool sign, uint64_t skip_flag=0, _async=False):
    cdef vector[vector[action]] vv
    cdef vector[action] v
    cdef action act
    cdef permission_level per
    cdef vector[permission_level] pers
    '''
    cdef cppclass permission_level:
        uint64_t    actor;
        uint64_t permission;

    cdef cppclass action:
        uint64_t                    account;
        uint64_t                    name;
        vector[permission_level]    authorization;
        vector[char]                data;
    '''
    for aa in actions:
        v = vector[action]()
        for a in aa:
            act = action()
            act.account = a[0]
            act.name = a[1]
            pers = vector[permission_level]()
            for auth in a[2]:
                per = permission_level()
                per.actor = auth[0]
                per.permission = auth[1]
                pers.push_back(per)
            act.authorization = pers
            act.data.resize(0)
            act.data.resize(len(a[3]))
            memcpy(act.data.data(), a[3], len(a[3]))
            v.push_back(act)
        vv.push_back(v)
    return push_transactions2_(vv, sign, skip_flag, _async)

def mp_compile(py_file):
    cdef vector[char] buffer
    cdef int mpy_size
    cdef string s
    buffer.resize(0)
    buffer.resize(1024*64)
    src_data = None
    with open(py_file, 'r') as f:
        src_data = f.read()
    src_data = src_data.encode('utf8')
    file_name = os.path.basename(py_file)
    mpy_size = compile_and_save_to_buffer_(file_name, src_data, len(src_data), buffer.data(), buffer.size())
    s = string(buffer.data(), mpy_size)
    return <bytes>s

def hash64(data, uint64_t seed = 0):
   return XXH64(data, len(data), seed)

def wast2wasm( string& wast ):
    cdef string wasm
    wast2wasm_(wast, wasm)
    return <bytes>wasm

def is_replay():
    return is_replay_()

def pack_bytes(string& _in):
    cdef string out
    pack_bytes_(_in, out)
    return <bytes>out

def unpack_bytes(string& _in):
    cdef string out
    unpack_bytes_(_in, out)
    return <bytes>out

def pack_setabi(string& abiPath, uint64_t account):
    cdef string out
    fc_pack_setabi_(abiPath, account, out)
    return <bytes>out


