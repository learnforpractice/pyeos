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
                prepare('native', 'native.wast', 'native.abi', __file__)
            else:
                prepare('native', 'native.py', 'native.abi', __file__)
            return func(*args, **kwargs)
        return func_wrapper
    return init_decorator

@init()
def test(msg='hello,world'):
    with producer:
        r = eosapi.push_action('native', 'sayhello', msg, {'native':'active'})
        assert r

@init()
def deploy():
    sync = Sync('native', _dir=os.path.dirname(__file__), _ignore=['t.py', 'native.py'])
    sync.deploy_native('eosio.token', 0, '../contracts/eosio.token/libeosiotokend.dylib')

