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
    eosapi.push_action('hello', 'sayhello', 'hello,world', {'hello':'active'})

