import os
import time
import wallet
import eosapi
import initeos
import subprocess
import pickle

from common import prepare, producer, Sync

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('kitties', 'main.py', 'cryptokitties.abi', 2, __file__)
        return func(*args, **kwargs)
    return func_wrapper

_dir = os.path.dirname(os.path.abspath(__file__))

sync = Sync(_account = 'kitties', _dir = _dir, _ignore = ('main.py', 'test.py', 'ustruct.py', 'eoslib.py'))

@init
def deploy_depend_libs():
    from backyard import test
    test.deploy_mpy()

@init
def deploy_all():
    sync.deploy_all()

@init
def deploy(src_file):
    sync.deploy(src_file)

@init
def deploy_all_mpy():
    sync.deploy_all_mpy()

@init
def deploy_mpy(file_name):
    sync.deploy_mpy(file_name)

def clean():
    sync.clean()

@init
def test(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('kitties','sayhello',name,{'kitties':'active'})
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
        functions.append('call')
        arg = str(i)
        args.append(arg)
        contracts.append('kitties')
        per.append({'kitties':'active'})

    ret = eosapi.push_messages(contracts, functions, args, per, True)
    assert ret
    cost = ret['cost_time']
    eosapi.produce_block()
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

