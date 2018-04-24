import os
import time

import wallet
import eosapi
import initeos

from common import init_, producer

def init(func):
    def func_wrapper(*args):
        init_('auction1', 'simpleauction.py', 'simpleauction.abi', __file__)
        return func(*args)
    return func_wrapper

@init
def start(beneficiary='hello', auctionEnd=10000):
    with producer:
        beneficiary = eosapi.N(beneficiary)
        msg = int.to_bytes(beneficiary, 8, 'little')
        msg += int.to_bytes(auctionEnd, 8, 'little')
        r = eosapi.push_message('auction1','start', msg, {'auction1':'active'}, rawargs=True)
        assert r

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

@init
def create():
    with producer:
        msg = {"issuer":"eosio","maximum_supply":"1000000000.0000 EOS","can_freeze":0,"can_recall":0, "can_whitelist":0}
        r = eosapi.push_message('eosio.token', 'create', msg, {'eosio.token':'active'})
        assert r

@init
def issue():
    with producer:
        r = eosapi.push_message('eosio.token','issue',{"to":"hello","quantity":"100.0000 EOS","memo":""},{'eosio':'active'})
        assert r

@init
def transfer():
    with producer:
        msg = {"from":"eosio", "to":"auction1", "quantity":"1.0000 EOS", "memo":"m"}
        r = eosapi.push_message('eosio.token', 'transfer', msg, {'eosio':'active'})
        assert r


