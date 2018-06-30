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



def test():
    contracts_path = os.path.join(os.getcwd(), '..', 'contracts')
    sys.path.append(os.getcwd())
    account = 'eosio'
    path = 'eosio.system'

    print('account', account)
    if not eosapi.get_account(account):
        with producer:
            r = eosapi.create_account('eosio', account, key1, key2)
            assert r

    _path = os.path.join(contracts_path, path, path)

    print('+++++++++code update', account)
    wast = _path + '.wast'
    abi = _path + '.abi'
    with producer:
        r = eosapi.set_contract(account, wast, abi, 0)

def t2():
    msg = eosapi.pack_args('eosio', 'boost', {'account':'eosio'})
    r = eosapi.push_action('eosio','boost', msg, {'eosio':'active'})
    assert r

    msg = eosapi.pack_args('eosio', 'boost', {'account':'eosio.token'})
    r = eosapi.push_action('eosio','boost', msg, {'eosio':'active'})
    assert r



