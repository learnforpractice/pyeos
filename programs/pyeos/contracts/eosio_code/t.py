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
        prepare('eosio.code', 'eosio.code.py', 'eosio.code.abi', __file__)
        func(*args, **kwargs)
    return func_wrapper

@init
def test(name='mike'):
    r, cost = eosapi.push_action('eosio.code','sayhello', name, {'eosio.code':'active'})
    print('cost time:', cost)
#    print(eosapi.JsonStruct(r[0]))
#    eosapi.produce_block()

@init
def play():
    r = eosapi.push_action('eosio.code', 'play', '', {'eosio.code':'active'})
    assert r

@init
def test2(count=100):
    actions = []
    for i in range(count):
        action = ['eosio.code', 'sayhello', str(i), {'eosio.code':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions)
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

@init
def tt(count=500):
    actions = []
    for i in range(count):
        args = {"from":'eosio', "to":'eosio.ram', "quantity":'%.4f EOS'%(0.01,), "memo":'hello'}
        action = ['eosio.token', 'transfer', args, {'eosio':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions)
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

@init
def ttt(count=200):
    actions = []
    for i in range(count):
        args = {"from":'eosio', "to":'eosio.ram', "quantity":'%.4f EOS'%(0.01,), "memo":str(i)}
        args = eosapi.pack_args('eosio.token', 'transfer', args)
        action = ['eosio.token', 'transfer', args, {'eosio':'active'}]
        actions.append([action])

    ret, cost = eosapi.push_transactions(actions)
    print('total cost time:%.3f s, cost per action: %.3f ms, transaction per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

@init
def test3(count=100):
    actions = []
    for i in range(count):
        act = ['hello', 'sayhello', b'hello,world%d'%(i,), {'eosio.code':'active'}]
        actions.append([act])
    rr, cost = eosapi.push_transactions(actions, True)
    print('total cost time:%.3f s, cost per action: %.3f ms, transaction per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))


