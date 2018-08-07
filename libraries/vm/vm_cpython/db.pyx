from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "<stdint.h>":
    ctypedef unsigned long long uint64_t
    ctypedef int int32_t
    ctypedef unsigned int uint32_t

cdef extern from "<eosiolib_native/vm_api.h>": # namespace "eosio::chain":
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

    int32_t db_idx64_store(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const uint64_t* secondary);
    void db_idx64_update(int32_t iterator, uint64_t payer, const uint64_t* secondary);
    void db_idx64_remove(int32_t iterator);


    int db_idx_double_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const uint64_t& secondary, uint64_t& primary )
    int db_idx_double_find_primary( uint64_t code, uint64_t scope, uint64_t table, uint64_t& secondary, uint64_t primary )
    int db_idx_double_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  uint64_t& secondary, uint64_t& primary )
    int db_idx_double_upperbound( uint64_t code, uint64_t scope, uint64_t table,  uint64_t& secondary, uint64_t& primary )
    int db_idx_double_end( uint64_t code, uint64_t scope, uint64_t table )
    int db_idx_double_next( int iterator, uint64_t& primary  )
    int db_idx_double_previous( int iterator, uint64_t& primary )

    cdef cppclass vm_api:
       int32_t (*db_store_i64)(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id,  const char* data, uint32_t len);
       int32_t (*db_store_i64_ex)(uint64_t code, uint64_t scope, uint64_t table, uint64_t payer, uint64_t id,  const char* data, uint32_t len);
    
       void (*db_update_i64)(int32_t iterator, uint64_t payer, const char* data, uint32_t len);
       void (*db_remove_i64)(int32_t iterator);
    
       void (*db_update_i64_ex)( uint64_t scope, uint64_t payer, uint64_t table, uint64_t id, const char* buffer, size_t buffer_size );
       void (*db_remove_i64_ex)( uint64_t scope, uint64_t payer, uint64_t table, uint64_t id );
    
       int32_t (*db_get_i64)(int32_t iterator, char* data, uint32_t len);
       int32_t (*db_get_i64_ex)( int itr, uint64_t* primary, char* buffer, size_t buffer_size );
       const char* (*db_get_i64_exex)( int itr, size_t* buffer_size );
    
       int32_t (*db_next_i64)(int32_t iterator, uint64_t* primary);
       int32_t (*db_previous_i64)(int32_t iterator, uint64_t* primary);
       int32_t (*db_find_i64)(uint64_t code, uint64_t scope, uint64_t table, uint64_t id);
       int32_t (*db_lowerbound_i64)(uint64_t code, uint64_t scope, uint64_t table, uint64_t id);
       int32_t (*db_upperbound_i64)(uint64_t code, uint64_t scope, uint64_t table, uint64_t id);
       int32_t (*db_end_i64)(uint64_t code, uint64_t scope, uint64_t table);
    
       int32_t (*db_idx64_store)(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const uint64_t* secondary);
       void (*db_idx64_update)(int32_t iterator, uint64_t payer, const uint64_t* secondary);
       void (*db_idx64_remove)(int32_t iterator);
    
       int32_t (*db_idx64_next)(int32_t iterator, uint64_t* primary);
       int32_t (*db_idx64_previous)(int32_t iterator, uint64_t* primary);
       int32_t (*db_idx64_find_primary)(uint64_t code, uint64_t scope, uint64_t table, uint64_t* secondary, uint64_t primary);
       int32_t (*db_idx64_find_secondary)(uint64_t code, uint64_t scope, uint64_t table, const uint64_t* secondary, uint64_t* primary);
       int32_t (*db_idx64_lowerbound)(uint64_t code, uint64_t scope, uint64_t table, uint64_t* secondary, uint64_t* primary);
       int32_t (*db_idx64_upperbound)(uint64_t code, uint64_t scope, uint64_t table, uint64_t* secondary, uint64_t* primary);
       int32_t (*db_idx64_end)(uint64_t code, uint64_t scope, uint64_t table);
'''
int32_t (*db_idx_double_store)(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const float64_t* secondary);
void (*db_idx_double_update)(int32_t iterator, uint64_t payer, const float64_t* secondary);
void (*db_idx_double_remove)(int32_t iterator);
int32_t (*db_idx_double_next)(int32_t iterator, uint64_t* primary);
int32_t (*db_idx_double_previous)(int32_t iterator, uint64_t* primary);
int32_t (*db_idx_double_find_primary)(uint64_t code, uint64_t scope, uint64_t table, float64_t* secondary, uint64_t primary);
int32_t (*db_idx_double_find_secondary)(uint64_t code, uint64_t scope, uint64_t table, const float64_t* secondary, uint64_t* primary);
int32_t (*db_idx_double_lowerbound)(uint64_t code, uint64_t scope, uint64_t table, float64_t* secondary, uint64_t* primary);
int32_t (*db_idx_double_upperbound)(uint64_t code, uint64_t scope, uint64_t table, float64_t* secondary, uint64_t* primary);
int32_t (*db_idx_double_end)(uint64_t code, uint64_t scope, uint64_t table);
'''
cdef extern from "vm_cpython.h":
    vm_api& api();

def store_i64(scope, table, payer, id, buffer):
    api().db_store_i64(scope, table, payer, id, buffer, len(buffer))

def update_i64(int itr, uint64_t payer, buffer):
    api().db_update_i64(itr, payer, buffer, len(buffer))

def remove_i64(int itr):
    api().db_remove_i64(itr)

def get_i64( int iterator ):
    cdef size_t size
    ret = None
    size = api().db_get_i64( iterator, <char*>0, 0 )
    if size <= 0:
        return None

    buffer = bytes(size)
    size = api().db_get_i64( iterator, buffer, size )
    return buffer

def next_i64( int iterator):
    cdef uint64_t primary = 0
    itr = api().db_next_i64( iterator, &primary )
    return (itr, primary)

def previous_i64( int iterator ):
    cdef uint64_t primary = 0
    itr = api().db_previous_i64( iterator, &primary )
    return (itr, primary)

def find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return api().db_find_i64( code, scope, table, id )

def lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return api().db_lowerbound_i64( code, scope, table, id )

def upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ):
    return api().db_upperbound_i64( code, scope, table, id )

def end_i64( uint64_t code, uint64_t scope, uint64_t table ):
    return api().db_end_i64( code, scope, table )


def db_idx64_store(uint64_t scope, uint64_t table, uint64_t payer, uint64_t primary, uint64_t secondary):
    return api().db_idx64_store(scope, table, payer, primary, &secondary)

def db_idx64_update(int32_t iterator, uint64_t payer, uint64_t secondary):
    return api().db_idx64_update(iterator, payer, &secondary)

def db_idx64_remove(int32_t iterator):
    return api().db_idx64_remove(iterator);

def db_idx64_next(int32_t iterator):
    cdef uint64_t primary = 0
    itr = api().db_idx64_next(iterator, &primary)
    return (itr, primary)

def db_idx64_previous(int32_t iterator):
    cdef uint64_t primary = 0
    itr = api().db_idx64_previous(iterator, &primary)
    return (itr, primary)

def db_idx64_find_primary(uint64_t code, uint64_t scope, uint64_t table, uint64_t primary):
    cdef uint64_t secondary = 0
    itr = api().db_idx64_find_primary(code, scope, table, &secondary, primary)
    return (itr, secondary)

def db_idx64_find_secondary(uint64_t code, uint64_t scope, uint64_t table, uint64_t secondary):
    cdef uint64_t primary = 0
    itr = api().db_idx64_find_secondary(code, scope, table, &secondary, &primary);
    return (itr, primary)

def db_idx64_lowerbound(uint64_t code, uint64_t scope, uint64_t table):
    cdef uint64_t secondary = 0
    cdef uint64_t primary = 0
    itr = api().db_idx64_lowerbound(code, scope, table, &secondary, &primary)
    return (itr, primary, secondary)

def db_idx64_upperbound(uint64_t code, uint64_t scope, uint64_t table):
    cdef uint64_t secondary = 0
    cdef uint64_t primary = 0
    itr = api().db_idx64_upperbound(code, scope, table, &secondary, &primary)
    return (itr, primary, secondary)

def db_idx64_end(uint64_t code, uint64_t scope, uint64_t table):
    return api().db_idx64_end(code, scope, table)
