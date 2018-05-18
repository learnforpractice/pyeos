import os
import time
import struct

import wallet
import eosapi
import initeos

from eosapi import N
from tools import cpp2wast

from common import prepare, producer

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init(wasm=False):
    def init_decorator(func):
        def func_wrapper(*args, **kwargs):
            if wasm:
                prepare('storagetest', 'storagetest.wast', 'storagetest.abi', 0, __file__)
                return func(*args, **kwargs)
            else:
                prepare('storagetest', 'storagetest.py', 'storagetest.abi', __file__)
                return func(*args, **kwargs)
        return func_wrapper
    return init_decorator

@init()
def test(msg='hello,world', wasm=False):
    with producer:
        r = eosapi.push_action('storagetest', 'sayhello', msg, {'storagetest':'active'})
        assert r

@init()
def test2(count=100):
    actions = []
    for i in range(count):
        act = [N('storagetest'), N('sayhello'), [[N('storagetest'), N('active')]], b'hello,world%d'%(i,)]
        actions.append([act])
    cost_time = eosapi.push_transactions(actions, True)
    print(1.0/(cost_time/1e6/100.0), cost_time)

@init()
def test3(count=1000):
    actions = []
    for i in range(count):
        action = ['storagetest', 'sayhello', {'storagetest':'active'}, str(i)]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions, True)
    assert ret[0]
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()
