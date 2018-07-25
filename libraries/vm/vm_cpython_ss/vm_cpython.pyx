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

py_modules = {}

cdef extern int init_function_whitelist():
    for _dict in [db.__dict__, eoslib.__dict__, struct.__dict__, int.__dict__]:
        for k in _dict:
            v = _dict[k]
            if callable(v):
                print('+++++++++',v)
                whitelist_function_(v)
    whitelist = [    int.from_bytes, 
                     int.to_bytes, 
                     hasattr, 
                     getattr, 
                     setattr, 
                     getattr(globals()['__builtins__'], 'print'),
                 ]
    for bltin in whitelist:
        whitelist_function_(bltin)

def _get_code(uint64_t account):
    cdef string code
    get_code(account,  code)
    return code

ModuleType = type(db)

def new_module(name):
    return ModuleType(name)

class dummy:
    def apply(self, a,b,c):
        pass

'''
#define CALL_FUNCTION           131
#define CALL_FUNCTION_KW        141
#define CALL_FUNCTION_EX        142
#define SETUP_WITH              143
'''
opcodes = [131, 141, 142, 143]
opcode_blacklist = [False for i in range(255)]
for opcode in opcodes:
    opcode_blacklist[opcode] = True

def validate(code):
    for i in range(0, len(code), 2):
        opcode = code[i]
        if opcode_blacklist[opcode]:
            print('bad opcode ', opcode)
            return False
    return True

'''
def _load_module(account, code):
    try:
        module = new_module(eoslib.n2s(account))
        ret = builtin_exec_(code, module.__dict__, module.__dict__)
#        exec(code, module.__dict__)
#        del module.__dict__['__builtins__']
        if ret:
            py_modules[account] = module
        else:
            py_modules[account] = dummy()
        return module
    except Exception as e:
        enable_opcode_inspector_(0)
        print(e)
    return None
'''

def _load_module(account, code):
    enable_create_code_object_(1)
    try:
        name = eoslib.n2s(account)
        module = new_module(name)
        co = compile(code, name, 'exec')
        if validate(co.co_code):
            exec(co, module.__dict__)
            py_modules[account] = module
        else:
            py_modules[account] = dummy()
        return module
    except Exception as e:
        enable_opcode_inspector_(0)
        print(e)
    return None


cdef extern string get_c_string(object s):
    return s

cdef extern int cpython_setcode(uint64_t account, string& code):
    if account in py_modules:
        del py_modules[account]
    if _load_module(account, code):
        return 1
    return 0

cdef extern int cpython_apply(unsigned long long receiver, unsigned long long account, unsigned long long action):
    print("++++cpython_apply")
    try:
        apply = None
        if receiver in py_modules:
            apply = py_modules[receiver].apply
        else:
            code = _get_code(receiver)
            mod = _load_module(receiver, code)
            if mod:
                apply = mod.apply
        print('++++++++apply:', apply)
        if apply:
            enable_create_code_object_(0)
            enable_injected_apis_(1)
            apply(receiver, account, action)
            enable_create_code_object_(1)
            enable_injected_apis_(0)
            return 1;
        return 0
    except:
        enable_create_code_object_(1)
        enable_injected_apis_(0)
#        print(e)
    return 0
