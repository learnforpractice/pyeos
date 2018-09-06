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
        prepare('javatest', 'hello.py', 'hello.abi', __file__)
        func(*args, **kwargs)
    return func_wrapper

def test():
    account = 'javatest'
    if not eosapi.get_account(account):
        eosapi.create_account('eosio', account, initeos.key1, initeos.key2)

    src = 'hello'
    args = eosapi.pack_args("eosio", 'setcode', {'account':account,'vmtype':12, 'vmversion':0, 'code':src.encode('utf8').hex()})
    eosapi.push_action('eosio', 'setcode', args, {account:'active'})

