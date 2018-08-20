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
        prepare('softfloat', 'softfloat.py', 'softfloat.abi', __file__, 7)
        func(*args, **kwargs)
    return func_wrapper

@init
def test(name='mike'):
    r = eosapi.push_action('softfloat','sayhello', name, {'softfloat':'active'})
    assert r and not r['except']
    print('cost time:', r['cost'])
#    print(eosapi.JsonStruct(r[0]))
#    eosapi.produce_block()

@init
def test2(count=100):
    actions = []
    for i in range(count):
        action = ['softfloat', 'sayhello', str(i), {'softfloat':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions, True)
    assert ret and not ret['except']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

@init
def test3(count=100):
    actions = []
    for i in range(count):
        act = [N('softfloat'), N('sayhello'), [[N('softfloat'), N('active')]], b'hello,world%d'%(i,)]
        actions.append([act])
    r, cost_time = eosapi.push_transactions(actions, True)
    print(1e6/(cost_time/count), cost_time)
