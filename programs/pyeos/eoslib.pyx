from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "eoslib_.hpp":
    ctypedef unsigned long long uint64_t
    uint64_t string_to_name_( const char* str )
    
    void requireAuth_( uint64_t account );
    int readMessage_( string& buffer );
    void requireScope_(uint64_t account);
    void requireNotice_( uint64_t account );

cdef uint64_t toname(name):
    if type(name) == int:
        return name
    if type(name) == str:
        name = bytes(name,'utf8')
    return string_to_name_(name)

def requireAuth(name):
    cdef uint64_t name_
    name_ = toname(name)
    requireAuth_(name_)

def readMessage():
    cdef string buffer
    cdef int ret
    ret = readMessage_(buffer)
    return buffer

def requireScope(account):
    cdef uint64_t account_
    account_ = toname(account)
    requireScope_(account_)

def requireNotice(account):
    cdef uint64_t account_
    account_ = toname(account)
    requireNotice_(account_)



     
    