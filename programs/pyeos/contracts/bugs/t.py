import os
import time
import struct
import wallet
import eosapi
import initeos

from eosapi import N

from common import prepare, producer

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('bugs', 'bugs.py', 'bugs.abi', 2, __file__)
        return func(*args, **kwargs)
    return func_wrapper

@init
def t():
    with producer:
        r = eosapi.push_message('bugs','t1','',{'bugs':'active'})
        assert r
    eosapi.produce_block()

#test deeply recursive generators
@init
def t2():
    with producer:
        r = eosapi.push_message('bugs','t2','',{'bugs':'active'})
        assert r
    eosapi.produce_block()

@init
def test_module_memory_leak(sign=True):
    _src_dir = os.path.dirname(__file__)
    actions = []
    for i in range(10000):
        code = struct.pack('QBB', N('bugs'), 1, 0)
        with open(os.path.join(_src_dir, 'bugs.py'), 'rb') as f:
            code += eosapi.pack_bytes(b'\x00'+f.read() + b'\n1==(%d)'%(i+100,))
        act = [N('eosio'), N('setcode'), [[N('bugs'), N('active')]], code]
        cost_time = eosapi.push_transactions2([[act]], sign)
        if i % 50 == 0:
            eosapi.produce_block()

@init
def test_reentrant(sign=True):
    with producer:
        r = eosapi.push_message('bugs','t2','',{'bugs':'active'})
        assert not r
    eosapi.produce_block()

@init
def t3(count=100, sign=True):
#    tracemalloc.start()
#    snapshot1 = tracemalloc.take_snapshot()
    wast = '../../build/contracts/eosio.token/eosio.token.wast'
    key_words = b"hello,world"
    r = eosapi.set_contract('bugs', wast, '../../build/contracts/eosio.token/eosio.token.abi', 0)

    msg = {"issuer":"eosio","maximum_supply":"1000000000.0000 EOS","can_freeze":0,"can_recall":0, "can_whitelist":0}
    r = eosapi.push_message('bugs', 'create', msg, {'bugs':'active'})

    r = eosapi.push_message('bugs','issue',{"to":"bugs","quantity":"1000000.0000 EOS","memo":""},{'eosio':'active'})

    _src_dir = os.path.dirname(__file__)
    for i in range(count):
        actions = []
        #break the wasm cache
        key_words = b"hello,world"
        wast_file = os.path.join(_src_dir, '/Users/newworld/dev/pyeos/build/contracts/eosio.token/eosio.token.wast')
        with open(wast_file, 'rb') as f:
            data = f.read()
            replace_str = b"%d"%(int(time.time()),)
            replace_str.zfill(len(key_words))
            data = data.replace(key_words, replace_str)
            wasm = eosapi.wast2wasm(data)
            raw_code = eosapi.pack_bytes(wasm)

        code = struct.pack('QBB', N('bugs'), 0, 0)
        code += raw_code
        
        act = [N('eosio'), N('setcode'), [[N('bugs'), N('active')]], code]
        setabi = eosapi.pack_setabi('../../build/contracts/eosio.token/eosio.token.abi', eosapi.N('bugs'))
        setabi_action = [N('eosio'), N('setabi'), [[N('bugs'), N('active')]], setabi]
        actions.append([act, setabi_action])


        code = struct.pack('QBB', N('eosio.token'), 0, 0)
        code += raw_code
        act = [N('eosio'), N('setcode'), [[N('eosio.token'), N('active')]], code]
        setabi = eosapi.pack_setabi('../../build/contracts/eosio.token/eosio.token.abi', eosapi.N('eosio.token'))
        setabi_action = [N('eosio'), N('setabi'), [[N('eosio.token'), N('active')]], setabi]
        actions.append([act, setabi_action])

        print('&'*50)
        cost_time = eosapi.push_transactions2(actions, sign)
        
        print('*'*50)
        msg = {"from":"bugs", "to":"eosio", "quantity":"0.0001 EOS", "memo":"%d"%(i,)}
        r = eosapi.push_message('bugs', 'transfer', msg, {'bugs':'active'})

        print('='*20, 'cached module should be decreased by 1 as eosio.token load the same code as bugs')
        msg = {"from":"bugs", "to":"eosio", "quantity":"0.0001 EOS", "memo":"%d"%(i,)}
        r = eosapi.push_message('eosio.token', 'transfer', msg, {'bugs':'active'})

        if i % 50 == 0:
            cost_time = eosapi.produce_block()
    eosapi.produce_block()


#test __new__
@init
def t4():
    with producer:
        r = eosapi.push_message('bugs','t4','',{'bugs':'active'})
        assert r
#    eosapi.produce_block()
