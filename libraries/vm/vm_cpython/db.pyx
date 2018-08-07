from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "<stdint.h>":
    ctypedef unsigned long long uint64_t

cdef extern from "eoslib_.hpp": # namespace "eosio::chain":
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

def store_i64(scope, table, payer, id, buffer):
    db_store_i64_(scope, table, payer, id, buffer, len(buffer))

def update_i64(int itr, uint64_t payer, buffer):
    db_update_i64_(itr, payer, buffer, len(buffer))

def remove_i64(int itr):
    db_remove_i64_(itr)

def get_i64( int iterator ):
    cdef size_t size
    ret = None
    size = db_get_i64_( iterator, <char*>0, 0 )
    if size <= 0:
        return None

    buffer = bytes(size)
    size = db_get_i64_( iterator, buffer, size )
    return buffer

def next_i64( int iterator):
    cdef uint64_t primary = 0
    itr = db_next_i64_( iterator, &primary )
    return (itr, primary)

def previous_i64( int iterator ):
    cdef uint64_t primary = 0
    itr = db_previous_i64_( iterator, &primary )
    return (itr, primary)

def find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_find_i64_( code, scope, table, id )

def lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_lowerbound_i64_( code, scope, table, id )

def upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_upperbound_i64_( code, scope, table, id )

def end_i64( uint64_t code, uint64_t scope, uint64_t table ):
    return db_end_i64_( code, scope, table )
