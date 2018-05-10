import os
import time
import struct

import wallet
import eosapi
import eoslib
import initeos

from common import prepare, producer

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('greeter', 'greeter.py', 'greeter.abi', __file__)
        return func(*args, **kwargs)
    return func_wrapper

@init
def test(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('greeter', 'setgreeting', name, {'greeter':'active'})
        assert r

    with producer:
        r = eosapi.push_message('greeter', 'greeting', '' , {'greeter':'active'})
        assert r

@init
def test2(count=100):
    contracts = []
    functions = []
    args = []
    per = []
    for i in range(count):
        functions.append('setgreeting')
        arg = str(i)
        args.append(arg)
        contracts.append('greeter')
        per.append({'greeter':'active'})
    ret = eosapi.push_messages(contracts, functions, args, per, True)
    assert ret
    cost = ret['cost_time']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()

    code = eosapi.N('greeter')
    id = eosapi.hash64('greeting', 0)
    greeting = 0
    itr = eoslib.db_find_i64(code, code, code, id)
    if itr >= 0:
        greeting = eoslib.db_get_i64(itr)
        print(greeting[1:])


