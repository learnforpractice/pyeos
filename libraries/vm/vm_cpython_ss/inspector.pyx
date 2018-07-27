# cython: c_string_type=str, c_string_encoding=ascii

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.map cimport map

import db
import eoslib
import struct

cdef extern from "<stdint.h>":
    ctypedef unsigned long long uint64_t

cdef extern from "vm_cpython.h":
    void get_code(uint64_t account, string& code)

    void enable_injected_apis_(int enabled)
    void whitelist_function_(object func)
    void enable_opcode_inspector_(int enable);

    object builtin_exec_(object source, object globals, object locals);
    void inspect_set_status_(int status);
    void enable_create_code_object_(int enable);
    void set_current_account_(uint64_t account);

    int filter_attr(object v, object name);

function_whitelist = {}
cdef extern int init_function_whitelist():
    for _dict in [db.__dict__, eoslib.__dict__, struct.__dict__, int.__dict__]:
        for k in _dict:
            v = _dict[k]
            if callable(v):
                print('+++++++++',v)
                whitelist_function_(v)
                function_whitelist[v] = True
    whitelist = [    int.from_bytes, 
                     int.to_bytes, 
                     hasattr, 
                     getattr, 
                     setattr, 
                     getattr(globals()['__builtins__'], 'print'),
                 ]
    print('++++++++++++++int.from_bytes:', int.from_bytes)
    for bltin in whitelist:
        whitelist_function_(bltin)
        function_whitelist[bltin] = True

__current_module = None

def add_function_to_white_list(func):
    print('+++++++++++=add_function_to_white_list:', func)
    function_whitelist[func] = True

cdef extern string get_c_string(object s):
    return s

cdef extern int py_inspect_function(func):
#    print('++++py_inspect_function',func)
    if func in function_whitelist:
        return 1

    if type(func.__self__) in [str, list, int]:
        return 1

    return 0

ModuleType = type(db)

cdef extern int py_inspect_setattr(v, name):
#    print('++++++++py_inspect_setattr:',v, name)
    if name == '__init__':
        return 1
    if name.startswith('__'):
        return 0
    if ModuleType == type(v):
        if __current_module != v:
            return 0
    return 0

cdef extern int py_inspect_getattr(v, name):
#    print('+++++++++py_inspect_getattr:',v, name)
    if __current_module == v:
        return 1
    return filter_attr(v, name)

def enable_create_code_object(enable):
    enable_create_code_object_(enable)

def enable_injected_apis(enable):
    enable_injected_apis_(enable)

def enable_create_code_object(enable):
    enable_create_code_object_(enable)

def enable_injected_apis(enable):
    enable_injected_apis_(enable)

def set_current_module(mod):
    global __current_module
    __current_module = mod

def builtin_exec(object co, object globals, object locals):
    return builtin_exec_(co, globals, locals)
    

