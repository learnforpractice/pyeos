# cython: c_string_type=str, c_string_encoding=ascii
from libcpp.string cimport string

cdef extern from "<eosio/chain/micropython_interface.hpp>":
    void* execute_from_str(const char *str);

cdef extern from "<eosio/chain/evm_interface.hpp>":
    void evm_test_(string _code, string _data);

cdef extern from "../interface/debug_.hpp":
    void debug_test();
    void set_debug_mode(int mode);

    void run_code_(string code)


cdef extern from "py/gc.h":
    ctypedef int size_t 
    cdef struct gc_info_t:
        size_t total;
        size_t used;
        size_t free;
        size_t max_free;
        size_t num_1block;
        size_t num_2block;
        size_t max_block;
    void gc_info(gc_info_t *info);
    void gc_dump_info();
    void gc_collect()
    
def eval(const char* code):
    set_debug_mode(1)
    execute_from_str(code)
    set_debug_mode(0)

def collect():
    gc_collect()
    
def dump_info():
    gc_dump_info()

def evm_test(string _code, string _data):
    evm_test_(_code, _data)

def test():
    debug_test()

def run_code(string code):
    run_code_(code)
  
