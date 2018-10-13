import os
import re
import sys
import imp
import time
import struct

import rodb as db
import debug
import wallet
import eosapi
import initeos
import unittest
import traceback
import marshal

from eosapi import N, mp_compile, pack_bytes, pack_setabi, push_transactions
from common import prepare, producer

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('eosio.code', 'eosio_code.py', 'eosio_code.abi', __file__)
        func(*args, **kwargs)
    return func_wrapper

aa = b'''
def sayHello(name):
    print('hello', name)
'''

@init
def test():
    args = {'account':'hello', 'code_name':'sayhello', 'code_type':1, 'code': aa.hex()}
    r, cost = eosapi.push_action('eosio.code','setcode', args, {'hello':'active'})
    print('cost time:', cost)

@init
def test_import():
    r, cost = eosapi.push_action('eosio.code','sayhello', b'hello,world', {'hello':'active'})
    print('cost time:', cost)

@init
def test2(count=100):
    actions = []
    for i in range(count):
        action = ['eosio.code', 'sayhello', str(i), {'eosio.code':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions)
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

@init
def test3(count=100):
    actions = []
    for i in range(count):
        act = ['eosio.code', 'sayhello', b'hello,world%d'%(i,), {'eosio.code':'active'}]
        actions.append([act])
    rr, cost = eosapi.push_transactions(actions, True)
    if cost:
        print('total cost time:%.3f s, cost per action: %.3f ms, transaction per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

@init
def test_setcode1():
    hello_code = b'hello,world'
    args = {'account':'hello', 'code_name':'hello', 'code_type':1, 'code': hello_code.hex()}
    r, cost = eosapi.push_action('eosio.code','setcode', args, {'hello':'active'})
    print('cost time:', cost)
    code = N('eosio.code')
    scope = code
    table = N('hello')
    id = N('hello')
    itr = db.find_i64(code, scope, table, id)
    print(itr)
    assert itr >= 0
    co = db.get_i64(itr)
    print(co)
    assert co == hello_code

class EosioCodeTestCase(unittest.TestCase):
    def setUp(self):
        pass

    @unittest.expectedFailure
    def test_setcode(self):
        test_setcode1()

    def tearDown(self):
        pass

def ut():
    unittest.main(module=ut.__module__, exit=False)

