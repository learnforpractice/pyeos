import os
import time
import struct

import wallet
import eosapi
import initeos

from eosapi import N
from tools import cpp2wast

from common import smart_call, producer

def init(func):
    def func_wrapper(wasm=False, *args, **kw_args):
        if wasm:
            src_path = os.path.dirname(os.path.abspath(__file__))
            cpp2wast.set_src_path(src_path)
            if not cpp2wast.build('async.cpp'):
                raise Exception("building failed")
            return smart_call('async', 'sync.wast', 'sync.abi', 0, __file__, func, __name__, args)
        else:
            return smart_call('async', 'sync.py', 'async.abi', 2, __file__, func, __name__, args)
    return func_wrapper

@init
def test(msg='hello,world', wasm=False):
    with producer:
        r = eosapi.push_message('async', 'sayhello', msg, {'async':'active'}, rawargs=True)
        assert r


def gen_setabi_action():
    _src_dir = os.path.dirname(__file__)

    setabi = eosapi.pack_setabi(os.path.join(_src_dir, 'async.abi'), eosapi.N('async'))
    return [N('eosio'), N('setabi'), [[N('async'), N('active')]], setabi]

TRX_COUNT = 100

@init
def test_async(sign=True):
    _src_dir = os.path.dirname(__file__)
    actions = []
    for i in range(TRX_COUNT):
        code = struct.pack('QBB', N('async'), 1, 0)
        with open(os.path.join(_src_dir, 'async.py'), 'rb') as f:
            code += eosapi.pack_bytes(b'\x00' + f.read() + b'\n1==(%d)'%(i,))
        setcode_action = [N('eosio'), N('setcode'), [[N('async'), N('active')]], code]
        setabi_action = gen_setabi_action()
        callcode_action = [N('async'), N('sayhello'), [[N('async'), N('active')]], b'hello,world%d'%(i,)]
        actions.append([setcode_action, setabi_action, callcode_action])
    eosapi.push_transactions2(actions, sign, 0, True)

@init
def test_sync(sign=True):
    _src_dir = os.path.dirname(__file__)
    actions = []
    for i in range(100):
        code = struct.pack('QBB', N('async'), 1, 0)
        with open(os.path.join(_src_dir, 'async.py'), 'rb') as f:
            code += eosapi.pack_bytes(b'\x00' + f.read() + b'\n1==(%d)'%(i,))
        setcode_action = [N('eosio'), N('setcode'), [[N('async'), N('active')]], code]
        setabi_action = gen_setabi_action()
        callcode_action = [N('async'), N('sayhello'), [[N('async'), N('active')]], b'hello,world%d'%(i,)]
        actions.append([setcode_action, setabi_action, callcode_action])
    cost_time = eosapi.push_transactions2(actions, sign, 0, False)
    print(1.0/(cost_time/1e6/TRX_COUNT))


