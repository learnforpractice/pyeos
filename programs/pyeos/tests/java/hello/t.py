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
        prepare('hellojava', 'Hellojava.java', 'Hellojava.abi', __file__, 12)
        func(*args, **kwargs)
    return func_wrapper

@init
def test():
    r = eosapi.push_action('hellojava', 'sayhello', 'hello,worldddd', {'hellojava':'active'})
    print(r)

@init
def test2(count=100):
    actions = []
    for i in range(count):
        action = ['hellojava', 'sayhello', str(i), {'hellojava':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions)
    assert ret and not ret['except']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
