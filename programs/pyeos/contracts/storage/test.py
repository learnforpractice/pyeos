import os
import time
import struct

import wallet
import eosapi
import initeos

from eosapi import N
from tools import cpp2wast

from common import smart_call, producer

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init(func):
    def func_wrapper(wasm=False, *args, **kw_args):
        if wasm:
            src_path = os.path.dirname(os.path.abspath(__file__))
            cpp2wast.set_src_path(src_path)
            if not cpp2wast.build('storagetest.cpp'):
                raise Exception("building failed")
            return smart_call('storagetest', 'storagetest.wast', 'storagetest.abi', 0, __file__, func, __name__, args)
        else:
            return smart_call('storagetest', 'storagetest.py', 'storagetest.abi', 2, __file__, func, __name__, args)
    return func_wrapper

@init
def test(msg='hello,world', wasm=False):
    with producer:
        r = eosapi.push_message('storagetest', 'sayhello', msg, {'storagetest':'active'})
        assert r

@init
def test2(count=100):
    actions = []
    for i in range(count):
        act = [N('storagetest'), N('sayhello'), [[N('storagetest'), N('active')]], b'hello,world%d'%(i,)]
        actions.append([act])
    cost_time = eosapi.push_transactions2(actions, True)
    print(1.0/(cost_time/1e6/100.0), cost_time)

@init
def test3(count=100):
    contracts = []
    functions = []
    args = []
    per = []
    for i in range(count):
        functions.append('sayhello')
        arg = str(i)
        args.append(arg)
        contracts.append('storagetest')
        per.append({'storagetest':'active'})
    ret = eosapi.push_messages(contracts, functions, args, per, True)
    assert ret
    cost = ret['cost_time']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()
