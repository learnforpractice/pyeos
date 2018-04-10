# cython: c_string_type=bytes, c_string_encoding=ascii

from libcpp.string cimport string
from libcpp.vector cimport vector
from threading import Thread


cdef extern from "eoslib_.hpp":
    ctypedef unsigned long long uint64_t
    ctypedef int int32_t
    ctypedef unsigned int uint32_t

    int db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size )
    void db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size )
    void db_remove_i64( int itr )
    int db_get_i64( int itr, char* buffer, size_t buffer_size )
    int db_next_i64( int itr, uint64_t* primary )
    int db_previous_i64( int itr, uint64_t* primary )
    int db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id )
    int db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id )
    int db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id )
    int db_end_i64( uint64_t code, uint64_t scope, uint64_t table )


def store_i64( scope, table, payer, id , string buffer):
    return db_store_i64( scope, table, payer, id, buffer.c_str(), buffer.length() )

def update_i64( itr, payer, string buffer ):
    db_update_i64( itr, payer, buffer.c_str(), buffer.length())

def remove_i64( int itr ):
    db_remove_i64(itr)

def get_i64( int itr):
    cdef string _buffer
    cdef char buffer[512]
    cdef int size

    size = db_get_i64( itr, buffer, sizeof(buffer) )
    _buffer = string(buffer, size)
    return _buffer

def next_i64( int itr):
    cdef uint64_t primary
    cdef int itr_next
    itr_next = db_next_i64( itr, &primary )
    return (itr_next, primary)

def previous_i64( int itr ):
    cdef uint64_t primary
    cdef int itr_next
    itr_next = db_previous_i64( itr, &primary )
    return (itr_next, primary)

def find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_find_i64( code, scope, table, id )

def lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_lowerbound_i64( code, scope, table, id )

def upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_upperbound_i64( code, scope, table, id )

def end_i64( uint64_t code, uint64_t scope, uint64_t table ):
    return db_end_i64( code, scope, table )

