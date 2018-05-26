from libcpp.string cimport string
from libcpp.vector cimport vector

from eostypes_ cimport * 

cdef extern from "micropython/db_api.hpp":
    int db_api_get_i64( int itr, char* buffer, size_t buffer_size );
    int db_api_next_i64( int itr, uint64_t* primary );
    int db_api_previous_i64( int itr, uint64_t* primary );
    int db_api_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
    int db_api_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
    int db_api_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
    int db_api_end_i64( uint64_t code, uint64_t scope, uint64_t table );

def get_i64( int iterator ):
    cdef size_t size
    ret = None
    size = db_api_get_i64( iterator, <char*>0, 0 )
    if size <= 0:
        return None

    buffer = bytes(size)
    size = db_api_get_i64( iterator, buffer, size )
    return buffer

def next_i64( int iterator):
    cdef uint64_t primary = 0
    itr = db_api_next_i64( iterator, &primary )
    return (itr, primary)

def previous_i64( int iterator ):
    cdef uint64_t primary = 0
    itr = db_api_previous_i64( iterator, &primary )
    return (itr, primary)

def find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_api_find_i64( code, scope, table, id )

def lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_api_lowerbound_i64( code, scope, table, id )

def upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return db_api_upperbound_i64( code, scope, table, id )

def end_i64( uint64_t code, uint64_t scope, uint64_t table ):
    return db_api_end_i64( code, scope, table )