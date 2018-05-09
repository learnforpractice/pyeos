import os
import re
import sys
import imp
import time
import struct

import wallet
import eosapi
import initeos
import traceback
from common import prepare, producer

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('hello', 'hello.py', 'hello.abi', 2, __file__)
        func(*args, **kwargs)
    return func_wrapper

@init
def test(name='mike'):
    r = eosapi.push_message('hello','sayhello', name, {'hello':'active'})
    assert r
    eosapi.produce_block()

@init
def play():
    with producer:
        r = eosapi.push_message('hello', 'play', '', {'hello':'active'})
        assert r

@init
def test2(count=100):
    contracts = []
    functions = []
    args = []
    per = []
    for i in range(count):
        functions.append('sayhello')
        arg = str(i)
        args.append(arg)
        contracts.append('hello')
        per.append({'hello':'active'})
    ret = eosapi.push_messages(contracts, functions, args, per, True)
    assert ret
    cost = ret['cost_time']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()

@init
def deploy_mpy():
    src_dir = os.path.dirname(os.path.abspath(__file__))
    file_name = 'hello.py'
    
    src_code = eosapi.mp_compile(os.path.join(src_dir, file_name))
    file_name = file_name.replace('.py', '.mpy')
    mod_name = file_name
    msg = int.to_bytes(len(mod_name), 1, 'little')
    msg += mod_name.encode('utf8')
    msg += int.to_bytes(1, 1, 'little') # compiled code
    msg += src_code

    print('++++++++++++++++deply:', file_name)
    r = eosapi.push_message('kitties','deploy',msg,{'kitties':'active'})
    assert r

    producer.produce_block()
