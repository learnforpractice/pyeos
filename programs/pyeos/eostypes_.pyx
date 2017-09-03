#include <eos/types/generated.hpp>
from libcpp.string cimport string
cdef extern from  "<eos/types/PublicKey.hpp>":
    pass
cdef extern from  "<eos/types/Asset.hpp>":
    pass


cdef extern from "<vector>" namespace "std":
    cdef cppclass vector[T]:
        cppclass iterator:
            T operator*()
            iterator operator++()
            bint operator==(iterator)
            bint operator!=(iterator)
        vector()
        void push_back(T&)
        T& operator[](int)
        T& at(int)
        iterator begin()
        iterator end()

cdef extern from "<eos/types/native.hpp>" namespace "eos::types":
    ctypedef vector Vector
    ctypedef string String
    ctypedef Vector[char] Bytes
    cdef cppclass Name:        
        Name( const char* str ) except +
        String toString() const
        Name& operator=( const char* n )

cdef extern from "<eos/types/generated.hpp>" namespace "eos::types":
    ctypedef Name AccountName
    ctypedef Name PermissionName
    ctypedef Name FuncName

    cdef cppclass AccountPermission:
        AccountPermission();
        AccountPermission(const AccountName& account, const PermissionName& permission)
        AccountName                      account;
        PermissionName                   permission;

    cdef cppclass Message:
        Message()
        Message(const AccountName& code, const FuncName& type, const Vector[AccountPermission]& authorization, const Bytes& data)
        AccountName                      code;
        FuncName                         type;
        Vector[AccountPermission]        authorization;
        Bytes                            data;

cdef class PyMessage:
    cdef Message* msg      # hold a C++ instance which we're wrapping
    def __cinit__(self,code,funcName,authorization,data):
#        cdef AccountName code_
#        cdef FuncName funcName_
        cdef Vector[AccountPermission] authorization_
        cdef Bytes data_
        for a in authorization:
            account = bytes(a[0],'utf8')
            permission = bytes(a[1],'utf8')
            authorization_.push_back(AccountPermission(Name(account),Name(permission)))
        for d in bytearray(data,'utf8'):
            data_.push_back(<char>d)
        self.msg = new Message(AccountName(bytes(code,'utf8')),FuncName(bytes(funcName,'utf8')),authorization_,data_)
    def __dealloc__(self):
        del self.msg









