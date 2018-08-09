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

        int (*get_balance)(uint64_t _account, uint64_t _symbol, uint64_t* amount)
        int (*transfer)(uint64_t _from, uint64_t to, uint64_t _account, uint64_t _symbol);

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
    return name.decode('utf8')

def eosio_assert(cond, msg):
    if not cond:
        raise AssertionError(msg)

def now():
    return api().now()

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
    print(args)
    for key in permissions:
        per.actor = s2n(key)
        per.permission = s2n(permissions[key])
        _act.authorization.push_back(per)

    return send_inline_(_act)

def transfer(_from, _to, _amount, symbol=0):
    if isinstance(_from, str):
        _from = s2n(_from)

    if isinstance(_to, str):
        _to = s2n(_to)

    if not symbol:
        symbol=bytearray(8)
        symbol[0] = 4
        symbol[1] = ord('E')
        symbol[2] = ord('O')
        symbol[3] = ord('S')
        symbol, = struct.unpack('Q', symbol)
    return api().transfer(_from, _to, _amount, symbol)

#    return send_inline('eosio.token', 'transfer', args, {_from:'active'})

def get_balance(account, symbol=None):
    cdef uint64_t amount = 0

    if isinstance(account, str):
        account = s2n(account)
    if not symbol:
        symbol=bytearray(8)
        symbol[0] = 4
        symbol[1] = ord('E')
        symbol[2] = ord('O')
        symbol[3] = ord('S')
        symbol, = struct.unpack('Q', symbol)
    else:
        symbol, = struct.unpack('Q', symbol)
    api().get_balance(account, symbol, &amount)
    return amount

_code = N('eosio.token')

