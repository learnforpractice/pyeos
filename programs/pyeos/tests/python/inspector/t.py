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
        prepare('inspector', 'inspector.py', 'inspector.abi', __file__)
        func(*args, **kwargs)
    return func_wrapper

@init
def test(name='mike'):
    r, cost = eosapi.push_action('inspector','sayhello', name, {'inspector':'active'})
    print('cost time:', cost)


@init
def test2(name='mike'):
    acts = []
    act = ['inspector','sayhello', name, {'inspector':'active'}]
    for i in range(100):
        acts.append(act)
    r, cost = eosapi.push_actions(acts)
    print('cost time:', cost)


@init
def test3(name='mike'):
    acts = []
    for i in range(100):
        act = ['inspector','sayhello', str(i), {'inspector':'active'}]
        acts.append([act])
    r, cost = eosapi.push_transactions(acts)
    print('cost time:', cost)

