import sys
import struct
import logging
import importlib

import debug
import eoslib

sys.modules['ustruct'] = sys.modules['struct']

cdef extern int py_debug_apply(unsigned long long receiver, unsigned long long account, unsigned long long action) with gil:
    if receiver != debug.get_debug_contract():
        return 0;
    try:
        _account = eoslib.n2s(account)
        mod = importlib.import_module('{0}.{0}'.format(_account))
        mod.apply(receiver, account, action)
        return 1
    except Exception as e:
        logging.exception(e)
    return 0
