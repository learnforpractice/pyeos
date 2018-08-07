import os
import re
import sys
import imp
import time
import struct

import debug
import wallet
import eosapi
import initeos
import traceback
from eosapi import N, mp_compile, pack_bytes, pack_setabi, push_transactions
from common import prepare, producer

def init(func, wasm=False):
    def func_wrapper(*args, **kwargs):
        if not wasm:
            prepare('eosio.token', 'token.py', 'eosio.token.abi', __file__, 7)
        func(*args, **kwargs)
    return func_wrapper

@init
def create():
    msg = {"issuer":"eosio","maximum_supply":"11000000000000.0000 EOS"}
    r = eosapi.push_action('eosio.token', 'create', msg, {'eosio.token':'active'})
    print(r)

@init
def issue():
    r = eosapi.push_action('eosio.token','issue',{"to":"eosio","quantity":"10000000000.0000 EOS","memo":""},{'eosio':'active'})
    assert r

@init
def issue2():
    r = eosapi.push_action('eosio.token','issue',{"to":"hello","quantity":"10000000000.0000 EOS","memo":""},{'eosio':'active'})
    assert r

@init
def transfer():
    msg = {"from":"eosio", "to":"hello", "quantity":"1.0000 EOS", "memo":"m"}
    r = eosapi.push_action('eosio.token', 'transfer', msg, {'eosio':'active'})


@init
def test1(count=100):
    actions = []
    for i in range(count):
        action = ['eosio.token','issue',{"to":"eosio","quantity":"1.0000 EOS","memo":str(i)},{'eosio':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions)
    assert ret and not ret['except']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

@init
def test2(count=100):
    actions = []
    for i in range(count):
        action = ['eosio.token','transfer',{"from":"eosio", "to":"hello", "quantity":"0.0010 EOS", "memo":str(i)},{'eosio':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions)
    if ret['except']:
        print(ret['except'])
    assert not ret['except']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
