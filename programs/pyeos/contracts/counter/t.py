import os
import time
import wallet
import eosapi
import initeos
import rodb as db

import debug
from eosapi import N

from tools import cpp2wast
from common import prepare


def init(wasm=0):
    def init_decorator(func):
        def func_wrapper(*args, **kwargs):
            if wasm:
                prepare('counter', 'counter.wast', 'counter.abi', __file__)
                return func(*args, **kwargs)
            else:
                prepare('counter', 'counter.py', 'counter.abi', __file__, 6)
                return func(*args, **kwargs)
        return func_wrapper
    return init_decorator

@init()
def test(name=None):
    code = N('counter')
    counter_id = N('counter')
    counter_begin = 0
    itr = db.find_i64(code, code, code, counter_id)
    if itr >= 0:
        counter_begin = db.get_i64(itr)
        counter_begin = int.from_bytes(counter_begin, 'little')

    print('counter begin: ', counter_begin)

    r = eosapi.push_action('counter', 'count', '', {'counter':'active'})
    assert r and not r['except'] 

    counter_end = 0
    itr = db.find_i64(code, code, code, counter_id)
    if itr >= 0:
        counter_end = db.get_i64(itr)
        counter_end = int.from_bytes(counter_end, 'little')

    print('counter end: ', counter_end)

    assert counter_begin + 1 == counter_end

@init()
def test2(count=1000, msg='wasm'):
    import time
    import json

    code = N('counter')
    counter_id = N('counter')
    counter_begin = 0
    itr = db.find_i64(code, code, code, counter_id)
    if itr >= 0:
        counter_begin = db.get_i64(itr)
        counter_begin = int.from_bytes(counter_begin, 'little')

    print('counter begin: ', counter_begin)

    actions = []
    for i in range(count):
        action = ['counter', 'count', msg+':'+str(i), {'counter':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions)
    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

    counter_end = 0
    itr = db.find_i64(code, code, code, counter_id)
    if itr >= 0:
        counter_end = db.get_i64(itr)
        counter_end = int.from_bytes(counter_end, 'little')
    print('counter end: ', counter_end)

    itr = db.find_i64(code, code, code, N('msg'))
    if itr >= 0:
        msg = db.get_i64(itr)
    print('msg', msg)

def build_native():
    _src_dir = os.path.dirname(os.path.abspath(__file__))
    cpp2wast.set_src_path(_src_dir)
    cpp2wast.build_native('counter.cpp', 'counter', debug=False)
    lib_file = os.path.join(_src_dir, 'libcounter.dylib')
    debug.set_debug_contract('counter', lib_file)

