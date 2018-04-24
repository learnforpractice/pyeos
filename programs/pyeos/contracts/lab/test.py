import os
import sys
import time
import eosapi

from common import init_, producer, Sync
from tools import cpp2wast
src_path = os.path.dirname(os.path.abspath(__file__))
cpp2wast.set_src_path(src_path)

def init(func):
    def func_wrapper(*args, **kw_args):
        if 'wasm' in kw_args and kw_args['wasm']:
            cpp2wast.build('lab.cpp')
            init_('lab', 'lab.wast', 'lab.abi', __file__, 0)
        else:
            init_('lab', 'lab.py', 'lab.abi', __file__, 2)
        return func(*args)
    return func_wrapper

_dir = os.path.dirname(os.path.abspath(__file__))

sync = Sync(_account = 'lab', _dir = _dir, _ignore = ['lab.py'])

@init
def test(msg='hello,world', wasm=True):
    with producer:
        r = eosapi.push_message('lab', 'sayhello', msg, {'lab':'active'}, rawargs=True)
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

    ret = eosapi.push_messages(contracts, functions, args, per, True, rawargs=True)
    assert ret
    cost = ret['cost_time']
    eosapi.produce_block()
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

