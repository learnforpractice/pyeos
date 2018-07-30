from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.map cimport map
import os
import sys
import imp
import logging
import traceback

import db
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

cdef extern int cpython_apply(unsigned long long receiver, unsigned long long account, unsigned long long action):
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
