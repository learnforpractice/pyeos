import os
import re
import sys
import imp
import time
import struct

import debug
import wallet
import eosapi
import initeos
import traceback
from eosapi import N, mp_compile, pack_bytes, pack_setabi, push_transactions
from common import prepare, producer

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('inspector', 'inspector.py', 'inspector.abi', __file__, 7)
        func(*args, **kwargs)
    return func_wrapper

@init
def test(name='mike'):
    r = eosapi.push_action('inspector','sayhello', name, {'inspector':'active'})
    assert r and not r['except']
    print('cost time:', r['cost'])

