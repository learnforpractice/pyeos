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

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('mytoken', 'token.py', 'eosio.token.abi', __file__, 6)
        func(*args, **kwargs)
    return func_wrapper

@init
def test_issue():
    msg = {"issuer":"eosio","maximum_supply":"11000000000000.0000 EOS"}
    r = eosapi.push_action('mytoken', 'create', msg, {'mytoken':'active'})

@init
def test_create():
    r = eosapi.push_action('mytoken','issue',{"to":"eosio","quantity":"10000000000000.0000 EOS","memo":""},{'eosio':'active'})
    assert r

@init
def test_transfer():
    msg = {"from":"eosio", "to":"hello", "quantity":"100.0000 EOS", "memo":"m"}
    r = eosapi.push_action('mytoken', 'transfer', msg, {'eosio':'active'})

@init
def test2(count=100):
    actions = []
    for i in range(count):
        action = ['mytoken', 'sayhello', str(i), {'mytoken':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions, True)
    assert ret and not ret['except']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
