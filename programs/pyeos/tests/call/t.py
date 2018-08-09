import os
import time
import struct

import wallet
import eosapi
import initeos

from eosapi import N
from tools import cpp2wast

from common import prepare, producer
import debug as _debug

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init_debug(wasm=False):
    def init_decorator(func):
        def func_wrapper(*args, **kwargs):
            if wasm:
                _src_dir = os.path.dirname(os.path.abspath(__file__))
                cpp2wast.set_src_path(_src_dir)
                cpp2wast.build_native('call.cpp', 'call')
                lib_file = os.path.join(_src_dir, 'libcall.dylib')
                _debug.set_debug_contract('call', lib_file)
            return func(*args, **kwargs)
        return func_wrapper
    return init_decorator

def init(wasm=False):
    def init_decorator(func):
        def func_wrapper(*args, **kwargs):
            if wasm:
                prepare('call', 'call.wast', 'call.abi', __file__)
            else:
                prepare('call', 'call.py', 'call.abi', __file__, 6)
            return func(*args, **kwargs)
        return func_wrapper
    return init_decorator

@init()
def test(msg='hello,world\n'):
    r = eosapi.push_action('call', 'sayhello', msg, {'call':'active', 'eosio':'active'})
    assert r

@init()
def call(msg='hello,world\n'):
    _from = 'eosio'
    _to = 'hello'
    amount = 0.01
    msg = struct.pack('QQQ', eosapi.s2n(_from), eosapi.s2n(_to), int(0.01*10000))

    print(eosapi.get_balance(_from), eosapi.get_balance(_to))

    r = eosapi.push_action('call', 'call', msg, {'call':'active', 'eosio':'active'})

    print(eosapi.get_balance(_from), eosapi.get_balance(_to))

    assert r

@init()
@init_debug()
def debug(msg='hello,world\n'):
    r = eosapi.push_action('call', 'sayhello', msg, {'call':'active'})
    assert r

