import os
import time
import struct
import wallet
import eosapi
import initeos

from eosapi import N

from common import smart_call, producer

def init(func):
    def func_wrapper(*args, **kwargs):
        return smart_call('bugs', 'bugs.py', 'bugs.abi', 2, __file__, func, __name__, args, kwargs)
    return func_wrapper

@init
def t():
    with producer:
        r = eosapi.push_message('bugs','t1','',{'bugs':'active'},rawargs=True)
        assert r
    eosapi.produce_block()

#test deeply recursive generators
@init
def t2():
    with producer:
        r = eosapi.push_message('bugs','t2','',{'bugs':'active'},rawargs=True)
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
def call_wasm_with_setcode(count=100, sign=True):
    import tracemalloc
#    tracemalloc.start()
#    snapshot1 = tracemalloc.take_snapshot()
    wast = '../../build/contracts/eosio.token/eosio.token.wast'
    key_words = b"hello,world"
    r = eosapi.set_contract('bugs', wast, '../../build/contracts/eosio.token/eosio.token.abi',0)

    msg = {"issuer":"eosio","maximum_supply":"1000000000.0000 EOS","can_freeze":0,"can_recall":0, "can_whitelist":0}
    r = eosapi.push_message('bugs', 'create', msg, {'bugs':'active'})

    r = eosapi.push_message('bugs','issue',{"to":"bugs","quantity":"1000000.0000 EOS","memo":""},{'eosio':'active'})

    _src_dir = os.path.dirname(__file__)
    for i in range(count):
        actions = []
        code = struct.pack('QBB', N('bugs'), 0, 0)
        #break the wasm cache
        key_words = b"hello,world"
        wast_file = os.path.join(_src_dir, '/Users/newworld/dev/pyeos/build/contracts/eosio.token/eosio.token.wast')
        with open(wast_file, 'rb') as f:
            data = f.read()
            replace_str = b"ii%d"%(i,)
            replace_str.zfill(len(key_words))
            data = data.replace(key_words, replace_str)
            wasm = eosapi.wast2wasm(data)
            code += eosapi.pack_bytes(wasm)

        act = [N('eosio'), N('setcode'), [[N('bugs'), N('active')]], code]
        actions.append([act])
        cost_time = eosapi.push_transactions2(actions, sign)
        msg = {"from":"bugs", "to":"eosio", "quantity":"0.0001 EOS", "memo":"%d"%(i,)}
        r = eosapi.push_message('bugs', 'transfer', msg, {'bugs':'active'})
        if i % 50 == 0:
            cost_time = eosapi.produce_block()
    if 0:
        snapshot2 = tracemalloc.take_snapshot()
        top_stats = snapshot2.compare_to(snapshot1, 'lineno')
    
        print("[ Top 10 differences ]")
        for stat in top_stats[:10]:
            print(stat)

def toname(i):
    return chr(ord('A')+int(i%10)) + chr(ord('A')+int((i/10)%10)) + chr(ord('A')+int((i/100)%10)) 

@init
def deploy_contract(currency='bugs'):
    wast = '../../build/contracts/eosio.token/eosio.token.wast'
    key_words = b"hello,world"
    r = eosapi.set_contract(currency, wast, '../../build/contracts/eosio.token/eosio.token.abi',0)

    msg = {"issuer":"eosio","maximum_supply":"1000000000.0000 EOS","can_freeze":0,"can_recall":0, "can_whitelist":0}
    r = eosapi.push_message('bugs', 'create', msg, {'bugs':'active'})

    r = eosapi.push_message('bugs','issue',{"to":"bugs","quantity":"1000000.0000 EOS","memo":""},{'eosio':'active'})

    for i in range(1000):
        with open(wast, 'rb') as f:
            data = f.read()
            #data.find(key_words)
            replace_str = b"%d"%(i,)
            replace_str.zfill(len(key_words))
            #replace key works with custom words to break the effect of code cache mechanism
            data = data.replace(key_words, replace_str)
            with open('currency2.wast', 'wb') as f:
                f.write(data)
        r = eosapi.set_contract(currency, 'currency2.wast', '../../build/contracts/eosio.token/eosio.token.abi',0)
        msg = {"from":"bugs", "to":"eosio", "quantity":"0.0001 EOS", "memo":"%d"%(i,)}
        r = eosapi.push_message('bugs', 'transfer', msg, {'bugs':'active'})
        if i % 100 == 0:
            eosapi.produce_block()
    
    eosapi.produce_block()
