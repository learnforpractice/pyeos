# cython: c_string_type=bytes, c_string_encoding=ascii

from eostypes_ cimport * 

cdef extern from "wasm_.hpp" namespace "eosio::chain":
    int wasm_test_(string& code, string& func, vector[uint64_t]& args, uint64_t _account, uint64_t _action, vector[char]& data) {

def wasm_call(string& code, string& func, vector[uint64_t]& args, uint64_t _account, uint64_t _action, vector[char]& data):
    return wasm_test_(code, func, args, _account, _action, data)


