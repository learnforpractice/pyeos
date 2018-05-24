import sys
import struct
import logging

import eoslib

sys.modules['ustruct'] = sys.modules['struct']

cdef extern int contract_debug_apply(unsigned long long receiver, unsigned long long account, unsigned long long action) with gil:
    try:
        _account = eoslib.n2s(account)
        mod = __import__('{0}.{0}'.format(_account))
        getattr(mod, _account).apply(receiver, account, action)
        return 0
    except Exception as e:
        logging.exception(e)
    return -1
