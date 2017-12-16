# include <eos/types/generated.hpp>
from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "":
    ctypedef int INT_65  # use to satisfy cython
    ctypedef unsigned int uint32_t
    ctypedef unsigned long long uint64_t

cdef extern from  "<eos/types/public_key.hpp>":
    pass

cdef extern from "" namespace "fc":
    cdef cppclass array[T, N]:
        array()
#        T data[N]
        T data[0]

cdef extern from "" namespace "fc::ecc":
    ctypedef array[unsigned char, INT_65] compact_signature;

cdef extern from "" namespace "boost::filesystem":
    cdef cppclass path:
        path()
        operator = (string & s)

cdef extern from "<fc/crypto/sha256.hpp>" namespace "fc":
    cdef cppclass sha256:
        string str() const

cdef extern from "<fc/time.hpp>" namespace "fc":
    cdef cppclass time_point_sec:
        time_point_sec()
        time_point_sec(uint32_t seconds)

cdef extern from "<eos/types/native.hpp>" namespace "eosio::types":
    ctypedef unsigned short uint16  # fake type
    ctypedef unsigned int   uint32  # fake type

    ctypedef time_point_sec Time
    ctypedef compact_signature signature
    ctypedef sha256 generated_transaction_id_type

    ctypedef vector Vector
    ctypedef string String
    ctypedef Vector[char] Bytes
    cdef cppclass name:        
        name(const char * str) except +
        string to_string() const
        name & operator = (const char * n)


cdef extern from "" namespace "boost::container":
    cdef cppclass flat_set[T]:
        cppclass iterator:
            T operator * ()
            iterator operator + +()
            bint operator == (iterator)
            bint operator != (iterator)
        flat_set()
        void push_back(T &)
        T & operator[](int)
        T & at(int)
        iterator begin()
        iterator end()

cdef extern from "<eos/types/public_key.hpp>" namespace "fc":
    cdef cppclass variant:
        variant()
        string as_string()

cdef extern from "<eos/types/public_key.hpp>" namespace "eosio::types":
    cdef struct public_key:
        public_key(const string & base58str)
#        operator string() const

cdef extern from "<eos/types/public_key.hpp>" namespace "fc":
    void to_variant(const public_key & var, variant & vo);


cdef extern from "<eos/chain/types.hpp>" namespace "eosio::chain":
    ctypedef public_key public_key_type
    ctypedef sha256 chain_id_type

cdef extern from "<eos/types/generated.hpp>" namespace "eosio::types":
    ctypedef name account_name
    ctypedef name permission_name
    ctypedef name func_name

    cdef cppclass account_permission:
        account_permission();
        account_permission(const account_name & account, const permission_name & permission)
        account_name                      account;
        permission_name                   permission;

    cdef cppclass message:
        message()
        message(const account_name & code, const func_name & type, const Vector[account_permission] & authorization, const Bytes & data)
        account_name                      code;
        func_name                         type;
        Vector[account_permission]        authorization;
        Bytes                            data;

    cdef cppclass transaction:
        uint16                           ref_block_num;
        uint32                           ref_block_prefix;
        Time                             expiration;
        Vector[account_name]              scope;
        Vector[account_name]              read_scope;
        Vector[message]                  messages;

    cdef cppclass signed_transaction(transaction):
        Vector[signature]                signatures;

    cdef cppclass ProcessedSyncTransaction(transaction):
        vector[message_output] output;

    cdef cppclass ProcessedSyncTransaction(transaction):
        vector[message_output] output;

    cdef cppclass GeneratedTransaction(transaction):
        generated_transaction_id_type id;

cdef extern from "<eos/chain/transaction.hpp>" namespace "eosio::chain":
    cdef struct message_output:
        pass

    cdef struct NotifyOutput:
        account_name   name;
        message_output output;

    cdef struct message_output:
        vector[NotifyOutput]             notify;  # ///< accounts to notify, may only be notified once
        vector[ProcessedSyncTransaction] sync_transactions;  # ///< transactions generated and applied after notify
        vector[GeneratedTransaction]     async_transactions;  # ///< transactions generated but not applied


