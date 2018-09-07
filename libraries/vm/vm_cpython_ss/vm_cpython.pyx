# cython: c_string_type=str, c_string_encoding=ascii

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.map cimport map
from cpython.ref cimport PyObject

#import db
import eoslib
import struct
import _tracemalloc

#import vm
import inspector

cdef extern from "<stdint.h>":
    ctypedef unsigned long long uint64_t

cdef extern from "<Python.h>":
    ctypedef unsigned long size_t
    size_t PyGC_Collect();

    void PyErr_Fetch(PyObject **p_type, PyObject **p_value, PyObject **p_traceback);
    void PyErr_Clear();

cdef extern from "<inspector/inspector.h>":
    void enable_injected_apis(int enabled)
    void enable_opcode_inspector(int enable);

    void enable_filter_set_attr(int enable);
    void enable_filter_get_attr(int enable);

    void whitelist_function(object func)

    object builtin_exec_(object source, object globals, object locals);

    void inspect_set_status(int status);
    void enable_create_code_object(int enable);
    void set_current_account(uint64_t account);
#    void set_current_module(object mod);
    void enable_inspect_obj_creation(int enable);

cdef extern from "vm_cpython.h":
    void get_code(uint64_t account, string& code)


    int vm_cpython_apply(object mod, unsigned long long receiver, unsigned long long account, unsigned long long action);

    void Py_SetRecursionLimit(int new_limit)
    int Py_GetRecursionLimit()

    object vm_load_module(string& name, string& bytecode)
    object vm_load_codeobject(string& name, string& bytecodes)

cdef extern from "<eosiolib_native/vm_api.h>":
    cdef cppclass vm_api:
        const char* (*vm_cpython_compile)(const char *name, const char *code, int size, int *result_size)
        const char* (*load_code_ext)(uint64_t account, uint64_t code_name, size_t* code_size);
    vm_api* get_vm_api()

def _get_code(uint64_t account):
    cdef string code
    get_code(account,  code)
    return code

__current_module = None
py_modules = {}
py_imported_modules = {}

ModuleType = type(inspector)

def new_module(name):
    return ModuleType(name)

class dummy:
    def apply(self, a,b,c):
        pass

class _sandbox:
    def __enter__(self):

#        enable_opcode_inspector_(1)

        enable_injected_apis(1)
        enable_create_code_object(0)
        enable_filter_set_attr(1)
        enable_filter_get_attr(1)

    def __exit__(self, type, value, traceback):

#        enable_opcode_inspector_(0)

        enable_injected_apis(0)
        enable_create_code_object(1)
        enable_filter_set_attr(0)
        enable_filter_get_attr(0)

sandbox = _sandbox()


'''
#define CALL_FUNCTION           131
#define CALL_FUNCTION_KW        141
#define CALL_FUNCTION_EX        142
#define SETUP_WITH              143
#define DELETE_NAME              91
#define RAISE_VARARGS           130
'''

#define SETUP_EXCEPT            121
#define POP_EXCEPT               89

#opcode_blacklist = {143:True, 91:True, 130:True, 121:True, 89:True}
#opcode_blacklist = {121:True, 89:True}

opcode_blacklist = {91:True}

def validate(co):
    code = co.co_code
    for i in range(0, len(code), 2):
        opcode = code[i]
        if opcode in opcode_blacklist:
            raise Exception('bad opcode ', opcode)
            return False

    for const in co.co_consts:
        if type(const) == type(co):
            if not validate(const):
                return False

    return True

cdef extern object load_module_from_db(uint64_t account, uint64_t code_name):
    cdef const char* bytecodes = NULL;
    cdef size_t code_size = 0;
    cdef string _bytecodes;

    if code_name in py_imported_modules:
        return py_imported_modules[code_name]

    bytecodes = get_vm_api()[0].load_code_ext(account, code_name, &code_size)
    if code_size == 0:
        return None

    _bytecodes = string(bytecodes, code_size)

    try:
        name = eoslib.n2s(code_name)
        co = vm_load_codeobject(name, _bytecodes)
        module = type(eoslib)(name)
        exec(co, module.__dict__)
        py_imported_modules[code_name] = module
        return module
    except:
        pass
    return None

cdef object load_module(uint64_t account, string& bytecodes):
    cdef int code_size = 0
    cdef const char *result
    name = eoslib.n2s(account)
#    result = get_vm_api()[0].vm_cpython_compile(name, code, len(code), &code_size)
#    _code = string(result, code_size)

    try:
        enable_injected_apis(1)
        enable_create_code_object(1)
        enable_filter_set_attr(0)
        enable_filter_get_attr(0)
#        print(<bytes>(&bytecodes)[0])
        co = vm_load_codeobject(name, bytecodes)

#        co = compile(code, name, 'exec')
        ret = co
        if validate(co):
            py_modules[account] = co
        else:
            py_modules[account] = None
            ret = None
        return ret
    except Exception as e:
        print('vm.load_module', e)
    return None

cdef extern int cpython_setcode(uint64_t account, string& code): # with gil:
    set_current_account(account)
    if account in py_modules:
        del py_modules[account]
    ret = load_module(account, code)
    set_current_account(0)

    if ret:
        return 1
    return 0

cdef extern int cpython_apply(unsigned long long receiver, unsigned long long account, unsigned long long action) except -1: # with gil:
    cdef string code

    set_current_account(receiver)
    mod = None
    if receiver in py_modules:
        co = py_modules[receiver]
    else:
        get_code(receiver, code)
        bytecodes = <bytes>code
        co = load_module(receiver, bytecodes)
    if not co:
        return 0

    name = eoslib.n2s(receiver)
    module = new_module(name)
    inspector.set_current_module(module)

    _dict = module.__dict__

    limit = Py_GetRecursionLimit()
    Py_SetRecursionLimit(20)
    ret = 1
    error = 0

    _tracemalloc.start()
    builtin_exec_(co, _dict, _dict)

    enable_injected_apis(1);
    enable_create_code_object(1);
    enable_filter_set_attr(1);
    enable_filter_get_attr(1);
    enable_inspect_obj_creation(1);

    module.apply(receiver, account, action)

    enable_injected_apis(0);
    enable_create_code_object(1);
    enable_filter_set_attr(0);
    enable_filter_get_attr(0);
    enable_inspect_obj_creation(0);

    del module

    key = None
    while len(py_imported_modules) > 0:
        for _key in py_imported_modules:
            key = _key
            break
        del py_imported_modules[_key]

    _tracemalloc.stop()
    Py_SetRecursionLimit(limit)
    set_current_account(0)
    return ret

cdef extern int cpython_call(uint64_t receiver, uint64_t func) with gil:
    '''
    try:
        if receiver in py_modules:
            mod = py_modules[receiver]
        else:
            code = _get_code(receiver)
            mod = _load_module(receiver, code)
        if not mod:
            return 0
        _func = eoslib.n2s(func)
        _func = getattr(mod, _func)
        _func()
        return 1
    except Exception as e:
        logging.exception(e)
    '''
    return 0
