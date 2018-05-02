import os
import time
import struct
import wallet
import eosapi
import initeos

from eosapi import N

from common import smart_call, producer

def init(func):
    def func_wrapper(*args, **kwargs):
        return smart_call('bugs', 'bugs.py', 'bugs.abi', 2, __file__, func, __name__, args, kwargs)
    return func_wrapper

@init
def t():
    with producer:
        r = eosapi.push_message('bugs','t1','',{'bugs':'active'},rawargs=True)
        assert r
    eosapi.produce_block()

#test deeply recursive generators
@init
def t2():
    with producer:
        r = eosapi.push_message('bugs','t2','',{'bugs':'active'},rawargs=True)
        assert r
    eosapi.produce_block()

@init
def test_module_memory_leak(sign=True):
    _src_dir = os.path.dirname(__file__)
    actions = []
    for i in range(10000):
        code = struct.pack('QBB', N('bugs'), 1, 0)
        with open(os.path.join(_src_dir, 'bugs.py'), 'rb') as f:
            code += eosapi.pack_bytes(b'\x00'+f.read() + b'\n1==(%d)'%(i+100,))
        act = [N('eosio'), N('setcode'), [[N('bugs'), N('active')]], code]
        cost_time = eosapi.push_transactions2([[act]], sign)
        if i % 50 == 0:
            eosapi.produce_block()
