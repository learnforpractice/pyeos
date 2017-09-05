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

cdef extern from "<eos/types/PublicKey.hpp>" namespace "fc":
    cdef cppclass variant:
        variant()
        string as_string()

cdef extern from "<eos/types/PublicKey.hpp>" namespace "eos::types":
    cdef struct PublicKey:
        pass

cdef extern from "<eos/types/PublicKey.hpp>" namespace "fc":
    void to_variant(const PublicKey& var,  variant& vo);


cdef extern from "<eos/chain/Types.hpp>" namespace "eos::chain":
    ctypedef PublicKey public_key_type

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










