# cython: c_string_type=bytes, c_string_encoding=utf8
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp cimport bool

cdef extern from "<stdint.h>":
    ctypedef unsigned long long uint64_t

cdef extern from "<stdlib.h>":
    char * malloc(size_t size)
    void free(char* ptr)

cdef extern from "<fc/log/logger.hpp>":
    void ilog(char* log)
    void elog(char* log)

cdef extern from "eoslib_.hpp" namespace "eosio::chain":
    ctypedef unsigned long long uint128_t #fake define should be ctypedef __uint128_t uint128_t

    void get_code_( uint64_t account, string& code )
    bool is_account_( uint64_t account )
    uint64_t s2n_(const char* str);
    void n2s_(uint64_t n, string& result);
    void eosio_assert_(int condition, const char* str);

    int db_get_i64_( int iterator, char* buffer, size_t buffer_size )
    int db_next_i64_( int iterator, uint64_t& primary )
    int db_previous_i64_( int iterator, uint64_t& primary )
    int db_find_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id )
    int db_lowerbound_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id )
    int db_upperbound_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id )
    int db_end_i64_( uint64_t code, uint64_t scope, uint64_t table )

    int db_idx64_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const uint64_t& secondary, uint64_t& primary )
    int db_idx64_find_primary( uint64_t code, uint64_t scope, uint64_t table, uint64_t& secondary, uint64_t primary )
    int db_idx64_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  uint64_t& secondary, uint64_t& primary )
    int db_idx64_upperbound( uint64_t code, uint64_t scope, uint64_t table,  uint64_t& secondary, uint64_t& primary )
    int db_idx64_end( uint64_t code, uint64_t scope, uint64_t table )
    int db_idx64_next( int iterator, uint64_t& primary  )
    int db_idx64_previous( int iterator, uint64_t& primary )
    

    
    int db_idx_double_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const uint64_t& secondary, uint64_t& primary )
    int db_idx_double_find_primary( uint64_t code, uint64_t scope, uint64_t table, uint64_t& secondary, uint64_t primary )
    int db_idx_double_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  uint64_t& secondary, uint64_t& primary )
    int db_idx_double_upperbound( uint64_t code, uint64_t scope, uint64_t table,  uint64_t& secondary, uint64_t& primary )
    int db_idx_double_end( uint64_t code, uint64_t scope, uint64_t table )
    int db_idx_double_next( int iterator, uint64_t& primary  )
    int db_idx_double_previous( int iterator, uint64_t& primary )
    
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
def get_code(uint64_t account):
    cdef string code
    get_code_( account, code )
    return code

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

def db_get_i64( int iterator ):
    cdef char* buffer
    cdef size_t size
    ret = None
    size = db_get_i64_( iterator, <char*>0, 0 )
    if size <= 0:
        return None

    buffer = <char*>malloc(size)
    size = db_get_i64_( iterator, buffer, size )
    ret = bytes(buffer[:size])
    free(buffer)
    return ret

def db_next_i64( int iterator):
    cdef uint64_t primary
    itr = db_next_i64_( iterator, primary )
    return (itr, primary)

def db_previous_i64( int iterator ):
    cdef uint64_t primary
    itr = db_previous_i64_( iterator, primary )
    return (itr, primary)

def db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_find_i64_( code, scope, table, id )

def db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_lowerbound_i64_( code, scope, table, id )

def db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_upperbound_i64_( code, scope, table, id )

def db_end_i64( uint64_t code, uint64_t scope, uint64_t table ):
    return db_end_i64_( code, scope, table )
