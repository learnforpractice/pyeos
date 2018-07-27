# cython: c_string_type=str, c_string_encoding=ascii

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.map cimport map

import db
import eoslib
import struct
import _tracemalloc

import vm
import inspector

cdef extern from "<stdint.h>":
    ctypedef unsigned long long uint64_t

cdef extern from "vm_cpython.h":
    void get_code(uint64_t account, string& code)

    void enable_injected_apis_(int enabled)
    void enable_opcode_inspector_(int enable);

    void enable_filter_set_attr_(int enable);
    void enable_filter_get_attr_(int enable);

    void whitelist_function_(object func)

    object builtin_exec_(object source, object globals, object locals);

    void inspect_set_status_(int status);
    void enable_create_code_object_(int enable);
    void set_current_account_(uint64_t account);
    void set_current_module_(object mod);
    
    

def _get_code(uint64_t account):
    cdef string code
    get_code(account,  code)
    return code

__current_module = None
py_modules = {}

ModuleType = type(inspector)

def new_module(name):
    return ModuleType(name)

class dummy:
    def apply(self, a,b,c):
        pass

class _sandbox:
    def __enter__(self):
        _tracemalloc.start()

#        enable_opcode_inspector_(1)

        enable_injected_apis_(1)
        enable_create_code_object_(0)
        enable_filter_set_attr_(1)
        enable_filter_get_attr_(1)

    def __exit__(self, type, value, traceback):
        _tracemalloc.stop()

#        enable_opcode_inspector_(0)

        enable_injected_apis_(0)
        enable_create_code_object_(1)
        enable_filter_set_attr_(0)
        enable_filter_get_attr_(0)

sandbox = _sandbox()

'''
#define CALL_FUNCTION           131
#define CALL_FUNCTION_KW        141
#define CALL_FUNCTION_EX        142
#define SETUP_WITH              143
'''
#opcodes = [131, 141, 142, 143]
opcodes = []
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

def load_module(account, code):
    print('++++load_module')
    try:
        name = eoslib.n2s(account)
        module = new_module(name)
        inspector.set_current_module(module)
        co = compile(code, name, 'exec')
        if validate(co.co_code):
            builtin_exec_(co, module.__dict__, module.__dict__)
            py_modules[account] = module
        else:
            py_modules[account] = dummy()
        return module
    except Exception as e:
        print('vm.load_module', e)
    return None

cdef extern int cpython_setcode(uint64_t account, string& code):
    set_current_account_(account)
    if account in py_modules:
        del py_modules[account]
    ret = load_module(account, code)

    if ret:
        return 1
    return 0

cdef extern int cpython_apply(unsigned long long receiver, unsigned long long account, unsigned long long action):
    set_current_account_(receiver)
    mod = None
    if receiver in py_modules:
        mod = py_modules[receiver]
    else:
        code = _get_code(receiver)
        mod = load_module(receiver, code)
    if not mod:
        return 0

    inspector.set_current_module(mod)

    ret = 0
    with sandbox:
        ret = vm.apply(mod, receiver, account, action)

    return ret
