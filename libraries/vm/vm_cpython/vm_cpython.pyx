from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.map cimport map
import os
import sys
import imp
import logging
import traceback
import importlib

import db
import debug
import eoslib

cdef extern from "<stdint.h>":
    ctypedef unsigned long long uint64_t

cdef extern from "vm_cpython.h":
    void get_code(uint64_t account, string& code)

sys.modules['ustruct'] = sys.modules['struct']

py_modules = {}

def _get_code(uint64_t account):
    cdef string code
    get_code(account,  code)
    return code

def _load_module(account, code):
    try:
        name = eoslib.n2s(account)
        co = compile(code, name, 'exec')
        module = imp.new_module(name)
        exec(co, module.__dict__)
        py_modules[account] = module
        return module
    except Exception as e:
        logging.exception(e)
    return None

cdef extern int cpython_setcode(uint64_t account, string& code):
    if _load_module(account, code):
        return 1
    return 0

def debug_apply(receiver, account, action):
    try:
        _account = eoslib.n2s(account)
        mod = importlib.import_module('{0}.{0}'.format(_account))
        mod.apply(receiver, account, action)
        return 1
    except Exception as e:
        logging.exception(e)
    return 0

cdef extern int cpython_apply(uint64_t receiver, uint64_t account, uint64_t action) with gil:
    if debug.get_debug_contract() == receiver:
        return debug_apply(receiver, account, action)
    try:
        if receiver in py_modules:
            module = py_modules[receiver]
            module.apply(receiver, account, action)
            return 1
        code = _get_code(receiver)
        module = _load_module(receiver, code)
        if not module:
            return 0
        module.apply(receiver, account, action)
        return 1
    except Exception as e:
        logging.exception(e)
    return 0

cdef extern int cpython_call(uint64_t receiver, uint64_t func) with gil:
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
    return 0



