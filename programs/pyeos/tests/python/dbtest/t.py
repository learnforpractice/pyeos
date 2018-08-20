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
                cpp2wast.build_native('db.cpp', 'db')
                lib_file = os.path.join(_src_dir, 'libdb.dylib')
                _debug.set_debug_contract('db', lib_file)
            return func(*args, **kwargs)
        return func_wrapper
    return init_decorator

def init(wasm=False):
    def init_decorator(func):
        def func_wrapper(*args, **kwargs):
            if wasm:
                prepare('db', 'db.wast', 'db.abi', __file__)
            else:
                prepare('db', 'db.py', 'db.abi', __file__, 6)
            return func(*args, **kwargs)
        return func_wrapper
    return init_decorator

@init()
def test(msg='hello,world\n'):
    r = eosapi.push_action('db', 'sayhello', msg, {'db':'active'})
    assert r

@init()
@init_debug()
def debug(msg='hello,world\n'):
    r = eosapi.push_action('db', 'sayhello', msg, {'db':'active'})
    assert r

