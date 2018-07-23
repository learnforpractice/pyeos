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
        prepare('hello', 'hello.py', 'hello.abi', __file__, 6)
        func(*args, **kwargs)
    return func_wrapper

@init
def test(name='mike'):
    r = eosapi.push_action('hello','sayhello', name, {'hello':'active'})
    assert r and not r['except']
    print('cost time:', r['cost'])
#    print(eosapi.JsonStruct(r[0]))
#    eosapi.produce_block()

def test2(name='mike'):
    account = 'hello'

    actions = []
    src_path = os.path.dirname(__file__)

    src_file = os.path.join(src_path, 'hello.py')
    abi_file = os.path.join(src_path, 'hello.abi')

    code = open(src_file, 'rb').read().hex()
    setcode = eosapi.pack_args('eosio', 'setcode', {'account':account,'vmtype':6, 'vmversion':0, 'code':code})
    setcode = ['eosio', 'setcode', setcode, {account:'active'}]
    actions.append(setcode)

    setabi = pack_setabi(abi_file, account)
    setabi = ['eosio', 'setabi', setabi, {account:'active'}]
    actions.append(setabi)

    act = ['hello','sayhello', name, {'hello':'active'}]
    actions.append(act)

    r, cost = eosapi.push_actions(actions)
    print(r)
    print('-------------')
    print(r['except'])
    print('cost time:', cost)
#    print(eosapi.JsonStruct(r[0]))
#    eosapi.produce_block()

