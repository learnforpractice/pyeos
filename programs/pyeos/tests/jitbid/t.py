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

def assert_ret(rr):
    for r in rr:
        if r['except']:
            print(r['except'])
        assert not r['except']

def b():
    account = 'lab'
    print('++++++boost account', account)
    msg = eosapi.pack_args('eosio', 'boost', {'account':account})
    act = ['eosio', 'boost', {'eosio':'active'}, msg]
    rr, cost = eosapi.push_actions([act])
    assert_ret(rr)

def cb():
    account = 'lab'
    print('++++++cancel boost account', account)
    msg = eosapi.pack_args('eosio', 'cancelboost', {'account':account})
    act = ['eosio', 'cancelboost', {'eosio':'active'}, msg]
    rr, cost = eosapi.push_actions([act])
    assert_ret(rr)

def t():
    msg = {'bidder':'lab', 'bid':"1.0000 EOS"}
    msg = eosapi.pack_args('eosio', 'bidjit', msg)
    act = ['eosio', 'bidjit', {'lab':'active'}, msg]

    rr, cost = eosapi.push_actions([act])
    assert_ret(rr)
