import os
import sys
import time
import eosapi

from common import prepare, producer, Sync

def init(func): 
    def func_wrapper(wasm=True, *args, **kwargs):
        if wasm:
            prepare('lab', 'lab.wast', 'lab.abi', 0, __file__)
            return func(*args, **kwargs)
        else:
            prepare('lab', 'lab.py', 'lab.abi', 2, __file__)
            return func(*args, **kwargs)
    return func_wrapper

_dir = os.path.dirname(os.path.abspath(__file__))

sync = Sync(_account = 'lab', _dir = _dir, _ignore = ['lab.py'])

@init
def test(msg='hello,world'):
    print('hello, world')
    with producer:
        r = eosapi.push_message('lab', 'sayhello', msg, {'lab':'active'})
        assert r

@init
def deploy():
    sync.deploy_all()

@init
def deploy_mpy():
    sync.deploy_all_mpy()

@init
def test2(count):
    import time
    import json
    contracts = []
    functions = []
    args = []
    per = []
    for i in range(count):
        functions.append('call')
        arg = str(i)
        args.append(arg)
        contracts.append('lab')
        per.append({'lab':'active'})

    ret = eosapi.push_messages(contracts, functions, args, per, True)
    assert ret
    cost = ret['cost_time']
    eosapi.produce_block()
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

