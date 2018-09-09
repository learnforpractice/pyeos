import os
import re
import sys
import imp
import time
import struct
import shlex
import subprocess

import debug
import wallet
import eosapi
import initeos
import traceback
from eosapi import N, mp_compile, pack_bytes, pack_setabi, push_transactions
from common import prepare, producer

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('hello', 'Hello.java', 'Hello.abi', __file__, 12)
        func(*args, **kwargs)
    return func_wrapper

@init
def test():
    eosapi.push_action('hello', 'sayhello', 'hello,worldddd', {'hello':'active'})

@init
def test2(count=100):
    actions = []
    for i in range(count):
        action = ['hello', 'sayhello', str(i), {'hello':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions)
    assert ret and not ret['except']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
