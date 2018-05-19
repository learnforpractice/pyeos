import os
import sys
import time
import eosapi

from common import prepare, producer, Sync

def init(wasm=True):
    def init_decorator(func):
        def func_wrapper(*args, **kwargs):
            if wasm:
                prepare('lab', 'lab.wast', 'lab.abi', __file__)
                return func(*args, **kwargs)
            else:
                prepare('lab', 'lab.py', 'lab.abi', __file__)
                return func(*args, **kwargs)
        return func_wrapper
    return init_decorator

_dir = os.path.dirname(os.path.abspath(__file__))

sync = Sync(_account = 'lab', _dir = _dir, _ignore = ['lab.py'])

@init(True)
def test(msg='hello,world'):
    with producer:
        r = eosapi.push_action('lab', 'sayhello', msg, {'lab':'active'})
        assert r

@init()
def deploy():
    sync.deploy_all()

@init()
def deploy_mpy():
    sync.deploy_all_mpy()

@init()
def test2(count):
    import time
    import json

    actions = []
    for i in range(count):
        action = ['lab', 'call', {'lab':'active'}, str(i)]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions, True)
    assert ret
    eosapi.produce_block()
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

