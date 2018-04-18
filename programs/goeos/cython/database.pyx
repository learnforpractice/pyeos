from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.map cimport map
from database_object cimport *

cdef extern from "":
    ctypedef unsigned long long uint64_t

cdef extern from "database_.hpp" namespace "python::database":
    object database_create(string& path);

def tobytes(ustr):
    if isinstance(ustr, bytes):
        return ustr
    if isinstance(ustr, str):
        ustr = bytes(ustr, 'utf8')
    return ustr

def create(path):
    path = tobytes(path)
    return database_create(path);


