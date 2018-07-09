import os
import sys
import time
import struct
import debug
import eosapi

from eosapi import N, push_transactions
from common import prepare, producer, Sync
from tools import cpp2wast

def init(wasm=True):
    def init_decorator(func):
        def func_wrapper(*args, **kwargs):
            if wasm:
                prepare('lab', 'lab.wast', 'lab.abi', __file__)
                return func(*args, **kwargs)
            else:
                prepare('lab', 'lab.py', 'lab.abi', __file__)
                return func(*args, **kwargs)
        return func_wrapper
    return init_decorator

_dir = os.path.dirname(os.path.abspath(__file__))

sync = Sync(_account = 'lab', _dir = _dir, _ignore = ['lab.py'])

@init(True)
def test(msg='hello,world'):
    with producer:
        r = eosapi.push_action('lab', 'sayhello', msg, {'lab':'active'})
        assert r

@init()
def deploy():
    sync.deploy_all()

@init()
def deploy_mpy():
    sync.deploy_all_mpy()

@init()
def test2(count=100):
    import time
    import json

    actions = []
    for i in range(count):
        action = ['lab', 'sayhello', {'lab':'active'}, str(i)]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions, True)
    assert ret
    eosapi.produce_block()
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

def set_contract(account, src_file, abi_file, vmtype=1, sign=True):
    '''Set code and abi for the account

    Args:
        account (str)    : account name
        src_file (str)   : source file path
        abi_file (str)   : abi file path
        vmtype            : virtual machine type, 0 for wasm, 1 for micropython, 2 for evm
        sign    (bool)    : True to sign transaction

    Returns:
        JsonStruct|None: 
    '''
    account = eosapi.N(account)
    code = struct.pack('QBB', account, vmtype, 0)

    if vmtype == 0:
        with open(src_file, 'rb') as f:
            wasm = eosapi.wast2wasm(f.read())
            code += eosapi.pack_bytes(wasm)

    setcode = [N('eosio'), N('setcode'), [[account, N('active')]], code]

    return push_transactions([[setcode]], sign, compress = True)

def build_native():
    _src_dir = os.path.dirname(os.path.abspath(__file__))
    cpp2wast.set_src_path(_src_dir)
    cpp2wast.build_native('lab.cpp', 'lab')
    lib_file = os.path.join(_src_dir, 'liblab.dylib')
    debug.set_debug_contract('lab', lib_file)
