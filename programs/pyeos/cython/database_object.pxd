# include <eos/types/generated.hpp>
from libcpp.string cimport string
from libcpp.vector cimport vector
include "eostypes_.pxd"

cdef extern from "":
    ctypedef int INT_65  # use to satisfy cython
    ctypedef unsigned int uint32_t
    ctypedef unsigned long long uint64_t
    ctypedef unsigned char uint8_t

cdef extern from "<eosio/chain/account_object.hpp>" namespace "eosio::chain":
    cppclass account_object :
#        id_type             id;
        name         account_name;
        uint8_t             vm_type #      = 0;
        uint8_t             vm_version #   = 0;
#        fc::sha256          code_version;
#        Time                creation_date;
#        shared_vector<char> code;
#        shared_vector<char> abi;

#        void set_abi( const eos::types::Abi& _abi );


