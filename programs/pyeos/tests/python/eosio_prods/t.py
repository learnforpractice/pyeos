import os
import time
import struct

import wallet
import eosapi
import initeos

from eosapi import N
from tools import cpp2wast

from common import prepare, producer

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init(wasm=True):
    def init_decorator(func):
        def func_wrapper(wasm=True, *args, **kwargs):
            eosapi.push_action('eosio', 'setpriv', {'account':'eosio.prods', 'is_priv':1}, {'eosio':'active'})
            r = eosapi.set_contract('eosio.prods', '../contracts/eosio.prods/eosio.prods.wast', '../contracts/eosio.prods/eosio.prods.abi', 0)
            return func(*args, **kwargs)
        return func_wrapper
    return init_decorator

@init()
def test(msg='hello,world'):
    msg = {"producer":"prod.aa", "account":"eosio"}
    r = eosapi.push_action('eosio.prods', 'votejit', {"producer":"prod.aa", "account":"eosio", 'last_code_update':0}, {'prod.aa':'active'})

@init()
def testclear(msg='hello,world'):
    msg = {"producer":"prod.aa", "account":"eosio"}
    r = eosapi.push_action('eosio.prods', 'clearjit', {"producer":"prod.aa", "account":"eosio"}, {'prod.aa':'active'})
