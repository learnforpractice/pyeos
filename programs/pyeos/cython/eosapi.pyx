# cython: c_string_type=str, c_string_encoding=ascii

import os
import sys
import time
import json
import struct
import signal
import atexit

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

cdef extern from "<eosio/chain/contract_types.hpp>" namespace "eosio::chain":
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
    int produce_block_()
    int produce_block_start_()
    int produce_block_end_()

    object get_info_()
    object get_block_(char* num_or_id)

    object get_account_(char* name)
    bool is_account_(const char* _name)

    object get_accounts_(char* public_key)
    object create_account_(string creator, string newaccount, string owner, string active, int sign)
    object get_controlled_accounts_(char* account_name);
    object create_key_()
    object get_public_key_(string& wif_key)

    object get_actions_(uint64_t account, int pos, int offset)
    object get_transaction_(string& id)

    object set_evm_contract_(string& eth_address, string& sol_bin, bool sign)

    int get_code_(string& name, string& wast, string& abi, string& code_hash, int & vm_type)
    int get_table_(string& scope, string& code, string& table, string& result)

    object get_currency_balance_(string& _code, string& _account, string& _symbol)

    int compile_and_save_to_buffer_(const char* src_name, const char *src_buffer, size_t src_size, char* buffer, size_t size)

    void wast2wasm_( string& wast ,string& result)
    void wasm2wast_(string& wasm, string& result)

    bool is_replay_()

    void pack_bytes_(string& _in, string& out)
    void unpack_bytes_(string& _in, string& out)

    cdef cppclass permission_level:
        permission_level()
        uint64_t    actor
        uint64_t permission

    cdef cppclass action:
        action()
        uint64_t                    account
        uint64_t                    name
        vector[permission_level]    authorization
        vector[char]                data

    object push_transactions_(vector[vector[action]]& actions, bool sign, uint64_t skip_flag, bool _async, bool _compress)
    void memcpy(char* dst, char* src, size_t len)
#    void fc_pack_setcode(setcode _setcode, vector<char>& out)

    void fc_pack_setconfig_(string& abiPath, uint64_t account, string& out);
    void fc_pack_setabi_(string& abiPath, uint64_t account, string& out)
    void fc_pack_updateauth(string& _account, string& _permission, string& _parent, string& _auth, uint32_t _delay, string& result)
    void fc_pack_args(uint64_t code, uint64_t action, string& js, string& bin) except +
    object fc_unpack_args(uint64_t code, uint64_t action, string& bin);

    object gen_transaction_(vector[action]& v, int expiration)
    object sign_transaction_(string& trx_json_to_sign, string& str_private_key)
    object push_raw_transaction_(string& signed_trx)

    string connect_(const string& host)
    string disconnect_(const string& host)
    object status_(const string& host);

    object connections_()

    void n_to_symbol_(uint64_t n, string& out)
    uint64_t symbol_to_n_(string& n)

    void zlib_compress_data_(const string& _in, string& out);
    void zlib_decompress_data_(const string& _int, string& out);

    bool debug_mode_()
    void start_eos_()
    void replay_block_(string& id, int s);

    void get_code_hash_(string& name, string& code_hash)
    void sha256_(string& data, string& hash)

    int has_option(const char* _option);
    int get_option(const char* option, char *result, int size);


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
#        return str(self.__dict__)
        return json.dumps(self, default=lambda x: x.__dict__, sort_keys=False, indent=4, separators=(',', ': '))

    def __repr__(self):
        return json.dumps(self, default=lambda x: x.__dict__, sort_keys=False, indent=4, separators=(',', ': '))

def s2n(string& name):
    ret = string_to_uint64_(name)
    if ret == 0:
        raise Exception('bad name {0}'.format(name))
    return ret

def n2s(n):
    if n < 0:
        n &= 0xffffffffffffffff
    return uint64_to_string_(n)

def N(name):
    return s2n(name)

def eos_name_to_eth_address(string name):
    return convert_to_eth_address(name)

def eth_address_to_eos_name(string addr):
    return convert_from_eth_address(addr)

def toobject(bstr):
    return JsonStruct(bstr)

def now():
    return now2_()

def produce_block():
    return

def produce_block_start():
    ret = produce_block_start_()
    return ret

def produce_block_end():
    ret = produce_block_end_()
    time.sleep(0.5)
    return ret

def get_info():
    info = get_info_()
    return JsonStruct(info)

def get_block(id):
    if isinstance(id, int):
        id = str(id)
    info = get_block_(id)
    if info:
        return JsonStruct(info)
    return None

def is_account(const char* name):
    if is_account_(name):
        return True
    return False

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

def get_currency_balance(string& _code, string& _account, string& _symbol = 'EOS'):
    return get_currency_balance_(_code, _account, _symbol)

def get_balance(account):
    ret = get_currency_balance('eosio.token', account, 'EOS')
    if ret:
        return float(ret[0].split(' ')[0])
    return 0.0

def transfer(_from, _to, _amount, _memo=''):
    args = {"from":_from, "to":_to, "quantity":'%.4f EOS'%(_amount,), "memo":_memo}
    r = push_action('eosio.token', 'transfer', args, {_from:'active'})
    if r and not r['except']:
        return True
    return False

def create_account(creator, account, owner_key, active_key, sign=True):
    actions = []
    args = {'creator': creator,
     'name': account,
     'owner': {'threshold': 1,
               'keys': [{'key': active_key,
                         'weight': 1}],
               'accounts': [],
               'waits': []},
     'active': {'threshold': 1,
                'keys': [{'key': owner_key,
                          'weight': 1}],
                'accounts': [],
                'waits': []}}
    try:
        push_action('eosio', 'newaccount', args, {creator:'active'})
    except:
        return False
    return True

def create_account2(creator, account, owner_key, active_key):
    actions = []
    args = {'creator': creator,
     'name': account,
     'owner': {'threshold': 1,
               'keys': [{'key': active_key,
                         'weight': 1}],
               'accounts': [],
               'waits': []},
     'active': {'threshold': 1,
                'keys': [{'key': owner_key,
                          'weight': 1}],
                'accounts': [],
                'waits': []}}

    act = ['eosio', 'newaccount', args, {creator:'active'}]
    actions.append(act)

    if get_code('eosio')[0]:
        args = {'payer':creator, 'receiver':account, 'bytes':64*1024}
        act = ['eosio', 'buyrambytes', args, {creator:'active'}]
        actions.append(act)

        args = {'from': creator,
         'receiver': account,
         'stake_net_quantity': '1.0050 EOS',
         'stake_cpu_quantity': '1.0050 EOS',
         'transfer': 1}
        act = ['eosio', 'delegatebw', args, {creator:'active'}]
        actions.append(act)

    rr, cost =  push_actions(actions)
    for r in rr:
        if r['except']:
            return False
    return True

def create_account3(creator, account, owner_key, active_key, net, cpu, ram):
    actions = []
    args = {'creator': creator,
     'name': account,
     'owner': {'threshold': 1,
               'keys': [{'key': active_key,
                         'weight': 1}],
               'accounts': [],
               'waits': []},
     'active': {'threshold': 1,
                'keys': [{'key': owner_key,
                          'weight': 1}],
                'accounts': [],
                'waits': []}}

    act = ['eosio', 'newaccount', args, {creator:'active'}]
    actions.append(act)

    if get_code('eosio')[0]:
        args = {'payer':'eosio', 'receiver':account, 'quant':"%.4f EOS"%(ram,)}
        act = ['eosio', 'buyram', args, {'eosio':'active'}]
        actions.append(act)

        args = {'from': creator,
         'receiver': account,
         'stake_net_quantity': '%.4f EOS'%(net,),
         'stake_cpu_quantity': '%.4f EOS'%(cpu,),
         'transfer': 1}
        act = ['eosio', 'delegatebw', args, {creator:'active'}]
        actions.append(act)

    rr, cost =  push_actions(actions)
    for r in rr:
        if r['except']:
            return False
    return True


def create_key():
    cdef string pub
    cdef string priv
    key = create_key_()
    return JsonStruct(key)

def get_public_key(priv_key):
    return get_public_key_(priv_key)

def get_actions(account, pos, offset):
    if isinstance(account, str):
        account = s2n(account)

    ret = get_actions_(account, pos, offset)
    if ret:
        return JsonStruct(ret)

def get_transaction(id):
    if isinstance(id, int):
        id = str(id)
    ret = get_transaction_(id)
    if ret:
        return JsonStruct(ret)
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
        produce_block_start_()
    
    def __exit__(self, type, value, traceback):
        produce_block_end_()
        time.sleep(0.5)

producer = Producer()

def mp_compile(py_file):
    '''Compile Micropython source to binary code.

    Args:
        py_file (str): Python source file path.

    Returns:
        bytes: Compiled code.
    '''
    cdef vector[char] buffer
    cdef int mpy_size
    cdef string s
    buffer.resize(0)
    buffer.resize(1024*1024)
    src_data = None
    with open(py_file, 'r') as f:
        src_data = f.read()
    src_data = src_data.encode('utf8')
    file_name = os.path.basename(py_file)
    mpy_size = compile_and_save_to_buffer_(file_name, src_data, len(src_data), buffer.data(), buffer.size())
    s = string(buffer.data(), mpy_size)
    return <bytes>s
    
def hash64(data, uint64_t seed = 0):
    '''64 bit hash using xxhash

    Args:
        data (str|bytes): data to be hashed
        seed (int): hash seed

    Returns:
        int: hash code in uint64_t
    '''

    return XXH64(data, len(data), seed)

def wast2wasm( string& wast ):
    '''Convert wast file to binary code

    Args:
        wast (bytes): wasm code in wast format

    Returns:
        bytes: binary wasm code.
    '''

    cdef string wasm
    wast2wasm_(wast, wasm)
    return <bytes>wasm

def wasm2wast( string& wasm ):
    cdef string wast
    wasm2wast_(wasm, wast)
    return <bytes>wast

def is_replay():
    ''' check if Eos is started with --replay

    Args: 

    Returns:
        bool: 
    '''

    return is_replay_()

def pack_bytes(string& _in):
    '''Pack bytes, equivalent to fc::raw::pack<string>(const fc::string& v) or
       fc::raw::pack<std::vector<char>>(const std::vector<char> v) in C++

    Args:
        _in (str|bytes): data to be packed, if str object is provided, 
            it will convert to bytes automatically with ascii encoding.

    Returns:
        bytes: Packed data.
    '''
    cdef string out
    pack_bytes_(_in, out)
    return <bytes>out

def unpack_bytes(string& _in):
    '''unpack bytes, equivalent to fc::raw::unpack<string>( const std::vector<char>& s )

    Args:
        _in (bytes): data to be unpacked,  

    Returns:
        bytes: Unpacked data.
    '''

    cdef string out
    unpack_bytes_(_in, out)
    return <bytes>out

def pack_setabi(string& abiPath, account):
    '''pack setabi struct

    Args:
        abiPath (str|bytes): abi file path
        account (int): account name encode in uint64_t

    Returns:
        bytes: packed set abi struct.
    '''

    cdef string out
    if isinstance(account, str):
        account = N(account)
    fc_pack_setabi_(abiPath, account, out)
    return <bytes>out

def pack_setconfig(string& configPath, account):
    '''pack setconfig struct

    Args:
        configPath (str|bytes): abi file path
        account (int): account name encode in uint64_t

    Returns:
        bytes: packed set config struct.
    '''
    if isinstance(account, str):
        account = N(account)

    cdef string out
    fc_pack_setconfig_(configPath, account, out)
    return <bytes>out

def pack_updateauth(string& _account, string& _permission, string& _parent, string& _auth, uint32_t _delay):
    cdef string result
    fc_pack_updateauth(_account, _permission, _parent, _auth, _delay, result)
    return <bytes>result

def pack_args(code, action, args):
    cdef string bin
    args = json.dumps(args)

    if isinstance(code, str):
        code = N(code)

    if isinstance(action, str):
        action = N(action)

    fc_pack_args(code, action, args, bin)
    if bin.empty():
        raise Exception('error ocurred in fc_pack_args')
    return <bytes>bin


def unpack_args(code, action, string& bin):
    if isinstance(code, str):
        code = N(code)

    if isinstance(action, str):
        action = N(action)

    return fc_unpack_args(code, action, bin);

def gen_transaction(actions, int expiration=100):
    cdef vector[action] v
    cdef action act
    cdef permission_level per
    cdef vector[permission_level] pers

    v = vector[action]()
    for a in actions:
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

    return gen_transaction_(v, expiration)

def sign_transaction(trx, string& str_private_key):
    if isinstance(trx, dict):
        trx = json.dumps(trx)

    return sign_transaction_(trx, str_private_key)

def push_raw_transaction(signed_trx):
    if isinstance(signed_trx, dict):
        signed_trx = json.dumps(signed_trx)
    return push_raw_transaction_(signed_trx)

def push_transactions(actions, sign = True, uint64_t skip_flag=0, _async=False, compress=False):
    '''Send transactions

    Args:
        actions (list): two dimension action list, structured in [[action1,action2, ...],[action1,action2,...]]
            each action represented in [account, name, [[actor1, permission1],[actor2, permission2]], data],
            according to C++ structure defined in transaction.hpp
           struct action {
              account_name               account;
              action_name                name;
              vector<permission_level>   authorization;
              bytes                      data;
            }
        sign (bool)     : whether to sign the transaction 
        skip_flag (int) : skip flag, default to 0,
            all flags are defined in enum validation_steps in eosio/chain/chain_controller.hpp
        _async          : default to False, True to send in asynchronized mode, 
            False to send in synchronized mode
    Returns:
        int: Sending transactions total cost time 
    '''

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
            act.account = N(a[0])
            act.name = N(a[1])
            pers = vector[permission_level]()
            for auth in a[3]:
                per = permission_level()
                per.actor = N(auth[0])
                per.permission = N(auth[1])
                pers.push_back(per)
            act.authorization = pers
            act.data.resize(0)
            act.data.resize(len(a[2]))
            memcpy(act.data.data(), a[2], len(a[2]))
            v.push_back(act)
        vv.push_back(v)

    if debug_mode_():
        produce_block_start_()
        ret = push_transactions_(vv, sign, skip_flag, _async, compress)
        time.sleep(0.5)
        produce_block_end_()
        return ret
    else:
        return push_transactions_(vv, sign, skip_flag, True, compress)

def push_action(contract, action, args, permissions: Dict, sign=True):
    '''Publishing message to blockchain

    Args:
        account (str)      : account name
        action (str)       : action name
        args (dict|bytes)  : action paramater, can be a dict or raw bytes
        abi_file (str)   : abi file path
        vmtype            : virtual machine type, 0 for wasm, 1 for micropython, 2 for evm
        sign    (bool)    : True to sign transaction

    Returns:
        JsonStruct|None: 
    '''
    assert type(args) in (str, dict, bytes)

    if isinstance(args, dict):
        args = pack_args(contract, action, args)

    pers = []
    for per in permissions:
        pers.append([per, permissions[per]])
    act = [contract, action, args, pers]
    outputs, cost_time = push_transactions([[act]], sign)
    if outputs:
        if outputs[0]['except']:
            raise Exception(outputs[0]['except'])
        outputs[0]['cost'] = cost_time
        return outputs[0]
    return None

def push_actions(actions, sign=True):
    _actions = []
    for act in actions:
        _act = [act[0], act[1]]

        args = act[2]
        if isinstance(args, dict):
            args = pack_args(_act[0], _act[1], args)
        _act.append(args)

        pers = []
        for per in act[3]:
            pers.append([per, act[3][per]])
        _act.append(pers)

        _actions.append(_act)

    return push_transactions([_actions], sign)


def push_evm_action(eth_address, args, permissions: Dict, sign=True):
    cdef string contract_

    contract_ = convert_from_eth_address(eth_address)
    print('===eth_address:', eth_address)
    print('===contract_:', contract_)

    pers = []
    for key in permissions:
        value = permissions[key]
        key = convert_from_eth_address(key)
        pers.append([key, value])

    if args[:2] == '0x':
        args = bytes.fromhex(args[2:])
    else:
        args = bytes.fromhex(args)
    act = [contract_, 'call', pers, args]
    outputs, cost_time = push_transactions([[act]], sign)
    if outputs:
        return (outputs[0], cost_time)
    return None


def set_contract(account, src_file, abi_file, vmtype=1, sign=True):
    '''Set code and abi for the account

    Args:
        account (str)    : account name
        src_file (str)   : source file path
        abi_file (str)   : abi file path
        vmtype            : virtual machine type, 0 for wasm, 1 for micropython, 2 for evm
        sign    (bool)    : True to sign transaction

    Returns:
        JsonStruct|None: 
    '''
    code = struct.pack('QBB', N(account), vmtype, 0)

    actions = []
    if vmtype == 0:
        with open(src_file, 'rb') as f:
            wasm = wast2wasm(f.read())
            code_hash = sha256(wasm)
            old_hash = get_code_hash(account)
            print(old_hash, code_hash)
            if code_hash != old_hash:
                code += pack_bytes(wasm)
                setcode = ['eosio', 'setcode', code, [[account, 'active']]]
                actions.append(setcode)
    elif vmtype == 1:
        mpy_code = b'\x01'
        mpy_code += mp_compile(src_file)
        code += pack_bytes(mpy_code)
        setcode = ['eosio', 'setcode', code, [[account, 'active']]]
        actions.append(setcode)
    else:
        raise Exception("unknown code")

    setabi = pack_setabi(abi_file, account)
    setabi = ['eosio', 'setabi', setabi, [[account, 'active']]]
    actions.append(setabi)

    ret, cost = push_transactions([actions], sign, compress = True)
    if ret:
        if ret[0]['except']:
            raise Exception(ret[0]['except'])
        ret[0]['cost'] = cost
        return ret[0]
    return None

def set_evm_contract(eth_address, sol_bin, sign=True):
    ilog("set_evm_contract.....")

    if sign:
        sign = 1
    else:
        sign = 0
    if sol_bin[0:2] == '0x':
        sol_bin = sol_bin[2:]
    return set_evm_contract_(eth_address, sol_bin, sign)

def n2symbol(uint64_t n):
    cdef string out
    n_to_symbol_(n, out)
    return out

def symbol2n(string& n):
    return symbol_to_n_(n)

def symbolprecision(n):
    return n&0xff

def zlib_compress_data(string& _in):
    cdef string _out
    zlib_compress_data_(_in, _out)
    return <bytes>_out

def zlib_decompress_data(string& data):
    cdef string _out
    zlib_decompress_data_(data, _out)
    return <bytes>_out

def debug_mode():
    return debug_mode_()

cdef extern void py_exit() with gil:
    exit()

def get_code_hash(string& name):
    cdef string hash
    get_code_hash_(name, hash)
    return hash

def sha256(string& data):
    cdef string hash
    sha256_(data, hash)
    return hash

def has_opt(option):
    return has_option(option)

def get_opt(option):
    a = bytes(128)
    n = get_option(option, a, len(a))
    return a[:n].decode('utf8')

