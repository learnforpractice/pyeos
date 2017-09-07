#include <eos/types/generated.hpp>
from libcpp.string cimport string
from libcpp.vector cimport vector
cdef extern from "":
    ctypedef int INT_65 #use to satisfy cython
    ctypedef unsigned int uint32_t
cdef extern from  "<eos/types/PublicKey.hpp>":
    pass

cdef extern from  "<eos/types/Asset.hpp>":
    pass

cdef extern from "" namespace "fc":
    cdef cppclass array[T,N]:
        array()
#        T data[N]
        T data[0]

cdef extern from "" namespace "fc::ecc":
    ctypedef array[unsigned char,INT_65] compact_signature;

cdef extern from "" namespace "boost::filesystem":
    cdef cppclass path:
        path()
        operator=(string& s)

cdef extern from "<fc/crypto/sha256.hpp>" namespace "fc":
    cdef cppclass sha256:
        string str() const

cdef extern from "<fc/time.hpp>" namespace "fc":
    cdef cppclass time_point_sec:
        time_point_sec()
        time_point_sec(uint32_t seconds)

cdef extern from "" namespace "boost::container":
    cdef cppclass flat_set[T]:
        cppclass iterator:
            T operator*()
            iterator operator++()
            bint operator==(iterator)
            bint operator!=(iterator)
        flat_set()
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
        PublicKey(const string& base58str)
#        operator string() const

cdef extern from "<eos/types/PublicKey.hpp>" namespace "fc":
    void to_variant(const PublicKey& var,  variant& vo);


cdef extern from "<eos/chain/Types.hpp>" namespace "eos::chain":
    ctypedef PublicKey public_key_type
    ctypedef sha256 chain_id_type

cdef extern from "<eos/types/native.hpp>" namespace "eos::types":
    ctypedef unsigned short UInt16 #fake type
    ctypedef unsigned int   UInt32 #fake type
    ctypedef time_point_sec Time
    ctypedef compact_signature Signature
    ctypedef sha256 generated_transaction_id_type

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

    cdef cppclass Transaction:
        UInt16                           refBlockNum;
        UInt32                           refBlockPrefix;
        Time                             expiration;
        Vector[AccountName]              scope;
        Vector[AccountName]              readscope;
        Vector[Message]                  messages;

    cdef cppclass SignedTransaction(Transaction):
        Vector[Signature]                signatures;

    cdef cppclass ProcessedSyncTransaction(Transaction):
        vector[MessageOutput] output;

    cdef cppclass ProcessedSyncTransaction(Transaction):
        vector[MessageOutput] output;

    cdef cppclass GeneratedTransaction(Transaction):
        generated_transaction_id_type id;

cdef extern from "<eos/chain/transaction.hpp>" namespace "eos::chain":
    cdef struct MessageOutput:
        pass

    cdef struct NotifyOutput:
        AccountName   name;
        MessageOutput output;

    cdef struct MessageOutput:
        vector[NotifyOutput]             notify; #///< accounts to notify, may only be notified once
        vector[ProcessedSyncTransaction] sync_transactions; #///< transactions generated and applied after notify
        vector[GeneratedTransaction]     async_transactions; #///< transactions generated but not applied


