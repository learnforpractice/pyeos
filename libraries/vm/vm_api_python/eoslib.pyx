# cython: c_string_type=str, c_string_encoding=ascii
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp cimport bool

import db
import struct

cdef extern from "<stdint.h>":
    ctypedef unsigned long long uint64_t
    ctypedef unsigned int       uint32_t
    ctypedef int                int32_t
    ctypedef long long          int64_t

cdef extern from "<stdlib.h>":
    void memcpy(char* dst, char* src, size_t len)
    char * malloc(size_t size)
    void free(char* ptr)

cdef extern from "<fc/log/logger.hpp>":
    void ilog(char* log)
    void elog(char* log)

cdef extern from "<fc/crypto/xxhash.h>":
    uint64_t XXH64(const char *data, size_t length, uint64_t seed);

cdef extern from "eoslib_.hpp": # namespace "eosio::chain":
    ctypedef unsigned long long uint128_t #fake define should be ctypedef __uint128_t uint128_t

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
    
    int db_idx_double_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const uint64_t& secondary, uint64_t& primary )
    int db_idx_double_find_primary( uint64_t code, uint64_t scope, uint64_t table, uint64_t& secondary, uint64_t primary )
    int db_idx_double_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  uint64_t& secondary, uint64_t& primary )
    int db_idx_double_upperbound( uint64_t code, uint64_t scope, uint64_t table,  uint64_t& secondary, uint64_t& primary )
    int db_idx_double_end( uint64_t code, uint64_t scope, uint64_t table )
    int db_idx_double_next( int iterator, uint64_t& primary  )
    int db_idx_double_previous( int iterator, uint64_t& primary )

    void pack_bytes_(string& _in, string& out);
    void unpack_bytes_(string& _in, string& out);

    int call_set_args_(string& args);
    int call_get_args_(string& args);

    uint64_t call_(uint64_t account, uint64_t func);
    int send_inline_(action& act);

    cdef cppclass vm_api:
        uint32_t (*read_action_data)( void* msg, uint32_t len );
        uint32_t (*action_data_size)();
        void (*require_recipient)( uint64_t name );
        void (*require_auth)( uint64_t name );
        void (*require_auth2)( uint64_t name, uint64_t permission );
        bool (*has_auth)( uint64_t name );
        bool (*is_account)( uint64_t name );
        void (*send_inline)(const char *serialized_action, size_t size);
        void (*send_context_free_inline)(char *serialized_action, size_t size);
        uint64_t  (*publication_time)();
        uint64_t (*current_receiver)();
        uint32_t (*get_active_producers)( uint64_t* producers, uint32_t datalen );



        int64_t (*get_permission_last_used)( uint64_t account, uint64_t permission );
        int64_t (*get_account_creation_time)( uint64_t account );

        void (*set_resource_limits)( uint64_t account, int64_t ram_bytes, int64_t net_weight, int64_t cpu_weight );
        void (*get_resource_limits)( uint64_t account, int64_t* ram_bytes, int64_t* net_weight, int64_t* cpu_weight );
        int64_t (*set_proposed_producers)( char *producer_data, uint32_t producer_data_size );
        bool (*is_privileged)( uint64_t account );
        void (*set_privileged)( uint64_t account, bool is_priv );
        void (*set_blockchain_parameters_packed)(char* data, uint32_t datalen);
        uint32_t (*get_blockchain_parameters_packed)(char* data, uint32_t datalen);
        void (*activate_feature)( int64_t f );

        uint64_t (*string_to_uint64)(const char* str);
        int32_t (*uint64_to_string)(uint64_t n, char* out, int size);

        void (*eosio_abort)();
        void (*eosio_assert)( uint32_t test, const char* msg );
        void (*eosio_assert_message)( uint32_t test, const char* msg, uint32_t msg_len );
        void (*eosio_assert_code)( uint32_t test, uint64_t code );
        void (*eosio_exit)( int32_t code );
        uint64_t  (*current_time)();
        uint32_t  (*now)();

    vm_api& api()

cdef extern from "eoslib_.hpp" namespace "eosio::chain":
    uint64_t wasm_call2_(uint64_t receiver, string& file_name, string& func, vector[uint64_t]& args, vector[char]& result);

'''
int db_idx128_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const uint128_t& secondary, uint64_t& primary )
int db_idx128_find_primary( uint64_t code, uint64_t scope, uint64_t table, uint128_t& secondary, uint64_t primary )
int db_idx128_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  uint128_t& secondary, uint64_t& primary )
int db_idx128_upperbound( uint64_t code, uint64_t scope, uint64_t table,  uint128_t& secondary, uint64_t& primary )
int db_idx128_end( uint64_t code, uint64_t scope, uint64_t table )
int db_idx128_next( int iterator, uint64_t& primary  )
int db_idx128_previous( int iterator, uint64_t& primary )

int db_idx256_find_secondary( uint64_t code, uint64_t scope, uint64_t table, array_ptr<const idx256> data, size_t data_len, uint64_t& primary )
int db_idx256_find_primary( uint64_t code, uint64_t scope, uint64_t table, array_ptr<idx256> data, size_t data_len, uint64_t primary )
int db_idx256_lowerbound( uint64_t code, uint64_t scope, uint64_t table, array_ptr<idx256> data, size_t data_len, uint64_t& primary )
int db_idx256_upperbound( uint64_t code, uint64_t scope, uint64_t table, array_ptr<idx256> data, size_t data_len, uint64_t& primary )
int db_idx256_end( uint64_t code, uint64_t scope, uint64_t table )
int db_idx256_next( int iterator, uint64_t& primary  )
int db_idx256_previous( int iterator, uint64_t& primary )
'''

def is_account(uint64_t account):
    return api().is_account(account)

def N(const char* _str):
    return api().string_to_uint64(_str);

def s2n(const char* _str):
    return api().string_to_uint64(_str);

def n2s(uint64_t n):
    cdef int size;
    size = api().uint64_to_string(n, NULL, 0)

    name = bytes(size)
    api().uint64_to_string(n, name, size)
    return str(name)

def eosio_assert(_cond, const char* str):
    cdef int cond
    if _cond:
        cond = 1
    else:
        cond = 0
    api().eosio_assert(cond, str)

def read_action():
    cdef int size
    size = api().action_data_size()
    buf = bytes(size)
    api().read_action_data(<char*>buf, size);
    return buf

def require_auth(uint64_t account):
    return api().require_auth(account)

def require_recipient(uint64_t account):
    return api().require_recipient(account)

def wasm_call2(uint64_t receiver, string& file_name, string& func, vector[uint64_t]& args):
    cdef vector[char] result
    return None #wasm_call2_(receiver, file_name, func, args, result);

def hash64(data, uint64_t seed = 0):
    '''64 bit hash using xxhash

    Args:
        data (str|bytes): data to be hashed
        seed (int): hash seed

    Returns:
        int: hash code in uint64_t
    '''

    return XXH64(data, len(data), seed)


def pack_bytes(string& _in):
    cdef string out
    pack_bytes_(_in, out)
    return <bytes>out

def unpack_bytes(string& _in):
    cdef string out
    unpack_bytes_(_in, out)
    return <bytes>out

def call_set_args(string& args):
    return call_set_args_(args)

def call_get_args():
    cdef string args
    call_get_args_(args)
    return <bytes>args

def call(uint64_t account, uint64_t func):
    return call_(account, func)

def send_inline(contract, act, args: bytes, permissions):
    cdef action _act
    cdef permission_level per

    if isinstance(contract, str):
        contract = s2n(contract)

    if isinstance(act, str):
        act = s2n(act)

    _act.account = contract
    _act.name = act
    _act.data.resize(len(args))

    memcpy(_act.data.data(), args, len(args))

    for key in permissions:
        per.actor = s2n(key)
        per.permission = s2n(permissions[key])
        _act.authorization.push_back(per)

    return send_inline_(_act)

def transfer(_from, _to, _amount, _memo=''):
    _from = s2n('eosio')
    _to = s2n('hello')
    symbol=bytearray(8)
    symbol[0] = 4
    symbol[1] = ord('E')
    symbol[2] = ord('O')
    symbol[3] = ord('S')
    args = struct.pack('QQQ8s%ds'%(len(_memo),), _from, _to, _amount, symbol, _memo)
    return send_inline('eosio.token', 'transfer', args, {_from:'active'})

_code = N('eosio.token')

def to_symbol_name(s):
    n = int.from_bytes(s, 'little')
    return n>>8

class Asset(object):
    def __init__(self, amount=0, symbol=None):
        self.amount = amount
        self.symbol = symbol

    def pack(self):
        buffer = bytearray(16)
        struct.pack_into('Q8s', buffer, 0, self.amount, self.symbol)
        return bytes(buffer)

    def unpack(cls, data):
        a = Asset()
        a.amount, a.symbol = struct.unpack('Q8s', data)
        return a

class multi_index(object):
    def __init__(self, code, scope, table_id):
        self.code = code
        self.scope = scope
        self.table_id = table_id

    def store(self, payer):
        it = db.find_i64(self.code, self.scope, self.table_id, self.get_primary_key())
        if it >= 0:
            db.update_i64(it, payer, self.pack())
        else:
            db.store_i64(self.scope, self.table_id, payer, self.get_primary_key(), self.pack())

    def load(self):
        it = db.find_i64(self.code, self.scope, self.table_id, self.get_primary_key())
        if it >= 0:
            data = db.get_i64(it)
            return self.unpack(data)
        return False

    def get_primary_key(self):
        raise Exception('should be implemented by subclass')

    def pack(self):
        raise Exception('should be implemented by subclass')

    def unpack(self):
        raise Exception('should be implemented by subclass')

class Singleton(multi_index):
    def __init__(self, code, scope, table_id):
        multi_index.__init__(code, scope, table_id)
        self.pk_value = table_id

    def get_primary_key(self):
        return self.pk_value

class Balance(multi_index):
    def __init__(self, owner, symbol):
        self.owner = owner
        self.a = Asset(0, symbol)
        self.symbol_name = to_symbol_name(symbol)

        table_id = N('accounts')
        multi_index.__init__(self, _code, self.owner, table_id)

        self.load()

    def add(self, amount, payer):
        self.a.amount += amount
        self.store(payer)

    def sub(self, amount):
        assert(self.a.amount >= amount)
        self.a.amount -= amount
        self.store(self.owner)

    def get_primary_key(self):
        return self.symbol_name

    def pack(self):
        buffer = bytearray(16)
        struct.pack_into('Q8s', buffer, 0, self.a.amount, self.a.symbol)
        return bytes(buffer)

    def unpack(self, data):
        self.a.amount, self.a.symbol = struct.unpack('Q8s', data)
