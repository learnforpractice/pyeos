import struct
from libcpp.vector cimport vector
from libcpp.string cimport string
include "eostypes_.pxd"

cdef extern from "common.hpp":
    void emplace_scope(const Name& scope, vector[Name]& scopes)

cdef class PyTransaction:
    cdef Transaction* _thisptr

    def __cinit__(self):
        self._thisptr = NULL

    def __dealloc__(self):
        if self._thisptr:
            del self._thisptr

    def __call__(self):
        return <uint64_t>self._thisptr

    def __init__(self, refBlocNum = 0, refBlockPrefix = 0, expiration = 0, scopes = None, messages = None):
        self._thisptr = new Transaction()
        self._thisptr.refBlockNum = refBlocNum
        self._thisptr.refBlockPrefix = refBlockPrefix
        self._thisptr.expiration = time_point_sec(expiration)
        
        if scopes:
            for scope in scopes:
                emplace_scope(Name(scope),self._thisptr.scope)

        if messages:
            for msg in messages:
                self.add_message(msg)

    def reqireScope(self, scope,read_only = False):
        if read_only:
            emplace_scope(Name(scope),self._thisptr.readscope)
        else:
            emplace_scope(Name(scope),self._thisptr.scope)

    def add_message(self, msg: list):
        cdef uint64_t ptr = msg()
        if msg:
            self._thisptr.messages.push_back((<Message*>ptr)[0])

cdef class PyMessage:
    cdef Message* _thisptr

    def __cinit__(self):
        self._thisptr = NULL

    def __dealloc__(self):
        if self._thisptr:
            del self._thisptr

    def __call__(self):
        return <uint64_t>self._thisptr

    def __init__(self, code: bytes, type: bytes, author: list, data: bytes):
        self._thisptr = new Message()
        self._thisptr.code = Name(code)
        self._thisptr.type = Name(type)
        
        for a in author:
            self._thisptr.authorization.push_back(AccountPermission(Name(a[0]), Name(a[1])))
        self._thisptr.data = data
    
    def require_permission(self, account: bytes, permission: bytes):
        self._thisptr.authorization.push_back(AccountPermission(Name(account), Name(permission)))



