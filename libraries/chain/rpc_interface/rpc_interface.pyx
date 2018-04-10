# cython: c_string_type=bytes, c_string_encoding=ascii
from libcpp.string cimport string
import eosserver

cdef extern int rpc_interface_apply(unsigned long long account, unsigned long long action, string& code):
#    _account = int.to_bytes(account, 8, 'little')
    ret = eosserver.apply(account, action, code)
    return ret

