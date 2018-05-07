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

def init(func):
    def func_wrapper(wasm=True, *args, **kw_args):
        if wasm:
            prepare('twitbot', 'twitbot.wast', 'twitbot.abi', 0, __file__)
            return func(*args, **kwargs)
        else:
            prepare('twitbot', 'twitbot.py', 'twitbot.abi', 2, __file__)
            return func(*args, **kwargs)
    return func_wrapper

@init
def test(msg='hello,world', wasm=True):
    '''
    with producer:
        r = eosapi.push_message('twitbot', 'sayhello', msg, {'twitbot':'active'})
        assert r
    '''
    msg = {"from":"eosio", "to":"hello", "quantity":"100.0000 EOS", "memo":"m"}
    r = eosapi.push_message('eosio.token', 'transfer', msg, {'eosio':'active'})

    with producer:
        msg = {"from":"hello", "to":"twitbot", "quantity":"1.0000 EOS", "memo":"m"}
        r = eosapi.push_message('eosio.token', 'transfer', msg, {'hello':'active'})



