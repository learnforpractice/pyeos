import os
import time

import wallet
import eosapi
import initeos

from common import init_, producer

def init(func):
    def func_wrapper(*args):
        init_('hello', 'hello.py', 'hello.abi', __file__)
        return func(*args)
    return func_wrapper

@init
def test(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('hello', 'sayhello', name, {'hello':'active'}, rawargs=True)
        assert r

@init
def play():
    with producer:
        r = eosapi.push_message('hello', 'play', '', {'hello':'active'}, rawargs=True)
        assert r

@init
def test2(count):
    import time
    import json
    
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
    ret = eosapi.push_messages(contracts, functions, args, per, True, rawargs=True)
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
    r = eosapi.push_message('kitties','deploy',msg,{'kitties':'active'},rawargs=True)
    assert r

    producer.produce_block()

def create():
    with producer:
        r = eosapi.push_message('eosio.token', 'create', {"to":"eosio", "quantity":"10000.0000 EOS", "memo":""},{'eosio':'active'})
        assert r

def issue():
    with producer:
        r = eosapi.push_message('eosio.token','issue',{"to":"hello","quantity":"1000.0000 EOS","memo":""},{'hello':'active'})
        assert r

def transfer():
    with producer:
        msg = {"from":"eosio", "to":"hello", "quantity":"25.0000 EOS", "memo":"m"}
        r = eosapi.push_message('eosio.token', 'transfer', msg, {'eosio':'active'})
        assert r

