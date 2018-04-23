import os
import time

import wallet
import eosapi
import initeos

from common import init_, producer

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init(func):
    def func_wrapper(*args):
        init_('auction1', 'simpleauction.py', 'simpleauction.abi', __file__)
        return func(*args)
    return func_wrapper

@init
def bid(price):
    with producer:
        sender = eosapi.N('hello')
        msg = int.to_bytes(sender, 8, 'little')
        msg += int.to_bytes(price, 8, 'little')
        r = eosapi.push_message('auction1','bid', msg, {'auction1':'active'}, rawargs=True)
        assert r

@init
def withdraw():
    with producer:
        sender = eosapi.N('hello')
        msg = int.to_bytes(sender, 8, 'little')
        r = eosapi.push_message('auction1', 'withdraw', msg, {'auction1':'active'}, rawargs=True)
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
        contracts.append('auction1')
        per.append({'auction1':'active'})
    ret = eosapi.push_messages(contracts, functions, args, per, True, rawargs=True)
    assert ret
    cost = ret['cost_time']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()

