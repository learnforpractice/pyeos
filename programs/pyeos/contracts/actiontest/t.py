import os
import time
import struct

import wallet
import eosapi
import initeos
from eosapi import N

from common import prepare, producer

def init_wasm(func):
    def func_wrapper(*args, **kwargs):
        prepare('actiontest', 'actiontest.wast', 'actiontest.abi', 0, __file__)
        func(*args, **kwargs)
    return func_wrapper

def init_py(func):
    def func_wrapper(*args, **kwargs):
        prepare('actiontest', 'actiontest.py', 'actiontest.abi', 1, __file__)
        func(*args, **kwargs)
    return func_wrapper

def init_mpy(func):
    def func_wrapper(*args, **kwargs):
        prepare('actiontest', 'actiontest.py', 'actiontest.abi', 2, __file__)
        func(*args, **kwargs)
    return func_wrapper

'''
cdef cppclass permission_level:
     uint64_t    actor;
     uint64_t permission;

cdef cppclass action:
     uint64_t                    account;
     uint64_t                    name;
     vector[permission_level]    authorization;
     vector[char]                data;
'''

'''
struct setcode {
   account_name                     account;
   uint8                            vmtype;
   uint8                            vmversion;
   bytes                            code;
'''

TRX_COUNT = 500

@init_py
def send_actions(sign=True):
    actions = []
    for i in range(TRX_COUNT):
        act = [N('actiontest'), N('sayhello'), [[N('actiontest'), N('active')]], b'hello,world%d'%(i,)]
        actions.append([act])
    cost_time = eosapi.push_transactions2(actions, sign)
    print(1.0/(cost_time/1e6/100.0))
    start = time.time()
    eosapi.produce_block()
    print('produce_block: ', time.time()-start)

    return cost_time

def gen_setcode_action_py(nonce):
    _src_dir = os.path.dirname(__file__)
    code = struct.pack('QBB', N('actiontest'), 1, 0)
    with open(os.path.join(_src_dir, 'actiontest.py'), 'rb') as f:
        code += eosapi.pack_bytes(b'\x00'+f.read() + b'\n1==(%d)'%(nonce,))
    act = [N('eosio'), N('setcode'), [[N('actiontest'), N('active')]], code]
    return act

def gen_setabi_action():
    _src_dir = os.path.dirname(__file__)

    setabi = eosapi.pack_setabi(os.path.join(_src_dir, 'actiontest.abi'), eosapi.N('actiontest'))
    return [N('eosio'), N('setabi'), [[N('actiontest'), N('active')]], setabi]

@init_py
def call_py_with_setcode(sign=True):
    _src_dir = os.path.dirname(__file__)
    actions = []
    for i in range(TRX_COUNT):
        code = struct.pack('QBB', N('actiontest'), 1, 0)
        with open(os.path.join(_src_dir, 'actiontest.py'), 'rb') as f:
            code += eosapi.pack_bytes(b'\x00'+f.read() + b'\n1==(%d)'%(i+100,))
        act = [N('eosio'), N('setcode'), [[N('actiontest'), N('active')]], code]

        setabi_action = gen_setabi_action()
        actions.append([act, setabi_action])

    cost_time = eosapi.push_transactions2(actions, sign)
    print(1e6/(cost_time/TRX_COUNT))
    start = time.time()
    eosapi.produce_block()
    print('produce_block: ', time.time()-start)

    return 1e6/(cost_time/TRX_COUNT)

@init_py
def call_py_without_setcode(sign=True):
    _src_dir = os.path.dirname(__file__)
    actions = []
    for i in range(TRX_COUNT):
        act = [N('actiontest'), N('sayhello'), [[N('actiontest'), N('active')]], b'hello,world%d'%(i,)]
        actions.append([act])

    cost_time = eosapi.push_transactions2(actions, sign)
    print(1e6/(cost_time/TRX_COUNT))

    start = time.time()
    eosapi.produce_block()
    print('produce_block: ', time.time()-start)

    return 1e6/(cost_time/TRX_COUNT)

@init_mpy
def call_mpy_with_setcode(sign=True):
    _src_dir = os.path.dirname(__file__)
    actions = []
    for i in range(TRX_COUNT):
        code = struct.pack('QBB', N('actiontest'), 1, 0)
        py_file = os.path.join(_src_dir, 'actiontest.py')
        with open(py_file, 'rb') as f:
            src_code = f.read()
        src_code += b'\n1==(%d)'%(i,)

        with open(py_file[:-3]+'2.py', 'wb') as f:
            f.write(src_code)

        mpy_code = eosapi.mp_compile(py_file[:-3]+'2.py')
        code += eosapi.pack_bytes(b'\x01' + mpy_code)

        act = [N('eosio'), N('setcode'), [[N('actiontest'), N('active')]], code]
        setabi_action = gen_setabi_action()
        actions.append([act, setabi_action])

    cost_time = eosapi.push_transactions2(actions, sign)

    start = time.time()
    eosapi.produce_block()
    print('produce_block: ', time.time()-start)

    print(1e6/(cost_time/TRX_COUNT))
    return 1e6/(cost_time/TRX_COUNT)

@init_mpy
def call_mpy_without_setcode(sign=True):
    _src_dir = os.path.dirname(__file__)
    actions = []
    for i in range(TRX_COUNT):
        act = [N('actiontest'), N('sayhello'), [[N('actiontest'), N('active')]], b'hello,world%d'%(i,)]
        actions.append([act])

    cost_time = eosapi.push_transactions2(actions, sign)

    start = time.time()
    eosapi.produce_block()
    print('produce_block: ', time.time()-start)

    print(1e6/(cost_time/TRX_COUNT))
    return 1e6/(cost_time/TRX_COUNT)


@init_wasm
def call_wasm_with_setcode(sign=True):
    _src_dir = os.path.dirname(__file__)
    actions = []
    for i in range(TRX_COUNT):
        code = struct.pack('QBB', N('actiontest'), 0, 0)

        #break the wasm cache
        key_words = b"hello,world"
        wast_file = os.path.join(_src_dir, 'actiontest.wast')
#        wast_file = os.path.join(_src_dir, '/Users/newworld/dev/pyeos/build/contracts/eosio.token/eosio.token.wast')
        with open(wast_file, 'rb') as f:
            data = f.read()
            replace_str = b"hhh%d"%(i,)
            replace_str.zfill(len(key_words))
            data = data.replace(key_words, replace_str)
            wasm = eosapi.wast2wasm(data)
            code += eosapi.pack_bytes(wasm)

        act = [N('eosio'), N('setcode'), [[N('actiontest'), N('active')]], code]
        setabi_action = gen_setabi_action()
        actions.append([act, setabi_action])

    cost_time = eosapi.push_transactions2(actions, sign)
    start = time.time()
    eosapi.produce_block()
    print('produce_block: ', time.time()-start)
    print(1e6/(cost_time/TRX_COUNT))

    return 1e6/(cost_time/TRX_COUNT)

@init_wasm
def call_wasm_without_setcode(sign=True):
    _src_dir = os.path.dirname(__file__)
    actions = []
    for i in range(TRX_COUNT):
        act = [N('actiontest'), N('sayhello'), [[N('actiontest'), N('active')]], b'hello,world%d'%(i,)]
        actions.append([act])

    cost_time = eosapi.push_transactions2(actions, sign)

    start = time.time()
    eosapi.produce_block()
    print('produce_block: ', time.time()-start)

    print(1e6/(cost_time/TRX_COUNT))
    return 1e6/(cost_time/TRX_COUNT)


def test_all(sign=True):
    costs = []
    ret = call_py_with_setcode(sign)
    costs.append(ret)

    ret = call_py_without_setcode(sign)
    costs.append(ret)

    ret = call_mpy_with_setcode(sign)
    costs.append(ret)

    ret = call_mpy_without_setcode(sign)
    costs.append(ret)

    ret = call_wasm_with_setcode(sign)
    costs.append(ret)

    ret = call_wasm_without_setcode(sign)
    costs.append(ret)

    print(costs)

#2326.1223540358224  3804.017041996348  2808.0816590146446  3760.3880720490356  1641.8204505155315  3212.02582468763  
