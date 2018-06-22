# cython: c_string_type=str, c_string_encoding=ascii
from libcpp.string cimport string
from libcpp cimport bool
from eostypes_ cimport * 
import os

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

    void wasm_debug_enable_(int enable)
    bool wasm_debug_enabled_()

    void set_debug_contract_(string& _account, string& path)
    int mp_is_account2(string& account)

    void wasm_enable_native_contract_(bool b)
    bool wasm_is_native_contract_enabled_()

    void mp_set_max_execution_time_(int _max)

    void app_set_debug_mode_(bool d)

cdef extern from "py/gc.h":
    ctypedef int size_t 
    cdef struct gc_info_t:
        size_t total
        size_t used
        size_t free
        size_t max_free
        size_t num_1block
        size_t num_2block
        size_t max_block

    void gc_info(gc_info_t *info)
    void gc_dump_info()
    void gc_collect()

def eval(const char* code):
    get_mpapi().set_debug_mode(1)
    get_mpapi().execute_from_str(code)
    get_mpapi().set_debug_mode(0)

def evm_test(string _code, string _data):
    raise Exception("Fix me please.")
#    evm_test_(_code, _data)

def test():
    debug_test()

def run_code(string code):
    pass
    #run_code_(code)

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

def enable_wasm_debug():
    wasm_debug_enable_(1)

def disable_wasm_debug():
    wasm_debug_enable_(0)

def is_wasm_debug_enabled():
    return wasm_debug_enabled_()

def set_debug_contract(_account, path):
    if _account or path:
        if not os.path.exists(path):
            raise Exception(path + " not exists")
        if not mp_is_account2(_account):
            raise Exception(path + " not exists")
    set_debug_contract_(_account, path)

def wasm_enable_native_contract(b):
    wasm_enable_native_contract_(b);

def wasm_is_native_contract_enabled():
    return wasm_is_native_contract_enabled_();

def mp_set_max_execution_time(_max):
    mp_set_max_execution_time_(_max)

def app_set_debug_mode(d):
    app_set_debug_mode_(d)

