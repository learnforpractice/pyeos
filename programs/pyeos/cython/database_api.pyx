# cython: c_string_type=bytes, c_string_encoding=utf8
from libcpp.vector cimport vector
from libcpp.string cimport string

cdef extern from "<stdint.h>":
    ctypedef unsigned long long uint64_t

cdef extern from "<fc/log/logger.hpp>":
    void ilog(char* log)
    void elog(char* log)

cdef extern from "database_api.hpp" namespace "eosio::chain":
    ctypedef unsigned long long uint128_t #fake define should be ctypedef __uint128_t uint128_t
    
    void get_code_( uint64_t account, string& code )
    int db_get_i64( int iterator, char* buffer, size_t buffer_size )
    int db_next_i64( int iterator, uint64_t& primary )
    int db_previous_i64( int iterator, uint64_t& primary )
    int db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id )
    int db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id )
    int db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id )
    int db_end_i64( uint64_t code, uint64_t scope, uint64_t table )

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

cdef char buffer[1024*128]

def get_i64( int iterator ):
#    cdef char buffer[256]
    cdef int size
    size = db_get_i64( iterator, buffer, sizeof(buffer) )
    if size > sizeof(buffer):
        size = sizeof(buffer)
        elog("buffer not enough!")
    return string(buffer,size)

def next_i64( int iterator):
    cdef uint64_t primary
    itr = db_next_i64( iterator, primary )
    return (itr, primary)

def previous_i64( int iterator ):
    cdef uint64_t primary
    itr = db_previous_i64( iterator, primary )
    return (itr, primary)

def find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_find_i64( code, scope, table, id )

def lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_lowerbound_i64( code, scope, table, id )

def upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_upperbound_i64( code, scope, table, id )

def end_i64( uint64_t code, uint64_t scope, uint64_t table ):
    return db_end_i64( code, scope, table )
