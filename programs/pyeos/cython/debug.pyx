# cython: c_string_type=str, c_string_encoding=ascii
from libcpp.string cimport string
from libcpp cimport bool

cdef extern from "<eosio/chain/micropython_interface.hpp>":
    void* execute_from_str(const char* str);

cdef extern from "<eosio/chain/evm_interface.hpp>":
    void evm_test_(string _code, string _data);

cdef extern from "../interface/debug_.hpp":
    cdef struct mpapi:
        void* (*execute_from_str)(const char* str)
        void (*set_debug_mode)(int mode)

    void debug_test();
    
    void run_code_(string code)
    mpapi& get_mpapi();

    void py_debug_enable_(int enable);
    bool py_debug_enabled_();

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
    get_mpapi().set_debug_mode(1)
    get_mpapi().execute_from_str(code)
    get_mpapi().set_debug_mode(0)

def evm_test(string _code, string _data):
    evm_test_(_code, _data)

def test():
    debug_test()

def run_code(string code):
    run_code_(code)

def enable():
    py_debug_enable_(1)
    import pydevd
    pydevd.settrace(suspend=False)

def disable():
    py_debug_enable_(0)
    import pydevd
    pydevd.stoptrace()

def is_enabled():
    return py_debug_enabled_()


