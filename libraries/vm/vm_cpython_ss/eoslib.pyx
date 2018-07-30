from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp cimport bool

# ccython: c_string_type=str, c_string_encoding=ascii

cdef extern from "<stdint.h>":
    ctypedef unsigned long long uint64_t

cdef extern from "<stdlib.h>":
    char * malloc(size_t size)
    void free(char* ptr)

cdef extern from "<fc/log/logger.hpp>":
    void ilog(char* log)
    void elog(char* log)

cdef extern from "<fc/crypto/xxhash.h>":
    uint64_t XXH64(const char *data, size_t length, uint64_t seed);

cdef extern from "eoslib_.hpp": # namespace "eosio::chain":
    ctypedef unsigned long long uint128_t #fake define should be ctypedef __uint128_t uint128_t

    bool is_account_( uint64_t account )
    uint64_t s2n_(const char* str);
    void n2s_(uint64_t n, string& result);

    void eosio_assert_(int condition, const char* str);

    int action_size_();
    int read_action_(char* memory, size_t size);

    void require_auth_(uint64_t account);
    void require_recipient_(uint64_t account);


    int db_store_i64_( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size )
    void db_update_i64_( int itr, uint64_t payer, const char* buffer, size_t buffer_size )
    void db_remove_i64_( int itr )


    int db_get_i64_( int iterator, char* buffer, size_t buffer_size )
    int db_next_i64_( int iterator, uint64_t* primary )
    int db_previous_i64_( int iterator, uint64_t* primary )
    int db_find_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id )
    int db_lowerbound_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id )
    int db_upperbound_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id )
    int db_end_i64_( uint64_t code, uint64_t scope, uint64_t table )

    int db_idx64_find_secondary_( uint64_t code, uint64_t scope, uint64_t table, const uint64_t& secondary, uint64_t& primary )
    int db_idx64_find_primary_( uint64_t code, uint64_t scope, uint64_t table, uint64_t& secondary, uint64_t primary )
    int db_idx64_lowerbound_( uint64_t code, uint64_t scope, uint64_t table,  uint64_t& secondary, uint64_t& primary )
    int db_idx64_upperbound_( uint64_t code, uint64_t scope, uint64_t table,  uint64_t& secondary, uint64_t& primary )
    int db_idx64_end_( uint64_t code, uint64_t scope, uint64_t table )
    int db_idx64_next_( int iterator, uint64_t& primary  )
    int db_idx64_previous_( int iterator, uint64_t& primary )
    

    
    int db_idx_double_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const uint64_t& secondary, uint64_t& primary )
    int db_idx_double_find_primary( uint64_t code, uint64_t scope, uint64_t table, uint64_t& secondary, uint64_t primary )
    int db_idx_double_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  uint64_t& secondary, uint64_t& primary )
    int db_idx_double_upperbound( uint64_t code, uint64_t scope, uint64_t table,  uint64_t& secondary, uint64_t& primary )
    int db_idx_double_end( uint64_t code, uint64_t scope, uint64_t table )
    int db_idx_double_next( int iterator, uint64_t& primary  )
    int db_idx_double_previous( int iterator, uint64_t& primary )

    void pack_bytes_(string& _in, string& out);
    void unpack_bytes_(string& _in, string& out);

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
    return is_account_(account)

def N(const char* _str):
    return s2n_(_str);

def s2n(const char* _str):
    return s2n_(_str);

def n2s(uint64_t n):
    cdef string result
    n2s_(n, result);
    return result

def eosio_assert(_cond, const char* str):
    cdef int cond
    if _cond:
        cond = 1
    else:
        cond = 0
    eosio_assert_(cond, str)

def read_action():
    cdef int size
    size = action_size_()
    buf = bytes(size)
    read_action_(buf, size);
    return buf

def require_auth(uint64_t account):
    return require_auth_(account)

def require_recipient(uint64_t account):
    return require_recipient_(account)

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
