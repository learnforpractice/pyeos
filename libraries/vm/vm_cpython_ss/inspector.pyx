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

    void enable_injected_apis(int enabled)
    void whitelist_function(object func)
    void enable_opcode_inspector(int enable);

    object builtin_exec_(object source, object globals, object locals);
    void inspect_set_status(int status);
    void enable_create_code_object(int enable);
    void set_current_account(uint64_t account);

    int filter_attr(object v, object name);

function_whitelist = {}
cdef extern int init_function_whitelist():
    for _dict in [db.__dict__, eoslib.__dict__, struct.__dict__, int.__dict__]:
        for k in _dict:
            v = _dict[k]
            if callable(v):
#                print('+++++++++',v)
                whitelist_function(v)
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
        whitelist_function(bltin)
        function_whitelist[bltin] = True

current_module = None
current_module_name = None

def add_function_to_whitelist(func):
#    print('+++++++++++=add_function_to_white_list:', func)
    function_whitelist[func] = True

cdef extern string get_c_string(object s):
    return s

cdef extern int py_inspect_function(func):
#    print('++++py_inspect_function',func)
    if func in function_whitelist:
        return 1

    if type(func.__self__) in [str, list, int, dict]:
        return 1

    return 0

ModuleType = type(db)

cdef extern int py_inspect_getattr(v, name):
#    print('+++++++++py_inspect_getattr:',v, name)
    if current_module == v:
        return 1
    return filter_attr(v, name)

def set_current_module(mod):
    global current_module
    global current_module_name
    current_module = mod
    current_module_name = current_module.__name__

cdef extern int cy_is_class_in_current_account(obj):
    global current_module_name
    if not hasattr(obj, '__module__'):
        return 0

    if  current_module_name == obj.__module__:
        return 1
    return 0
