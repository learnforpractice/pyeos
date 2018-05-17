import os
import time
import struct

import wallet
import eosapi
import initeos

from eosapi import N
from tools import cpp2wast

from common import prepare, Sync, producer

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init(wasm=False):
    def init_decorator(func):
        def func_wrapper(*args, **kwargs):
            if wasm:
                prepare('codestore', 'codestore.wast', 'codestore.abi', __file__)
                prepare('renter', 'renter.wast', 'renter.abi', __file__)
            else:
                prepare('codestore', 'codestore.py', 'codestore.abi', __file__)
                prepare('renter', 'renter.py', 'renter.abi', __file__)

            sync = Sync('codestore', _dir=os.path.dirname(__file__), _ignore=['t.py', 'renter.py', 'codestore.py'])
            sync.deploy_mpy('math.py')
            return func(*args, **kwargs)
        return func_wrapper
    return init_decorator

@init()
def test(msg='hello,world'):
    with producer:
        r = eosapi.push_message('renter', 'sayhello', msg, {'codestore':'active'})
        assert r
