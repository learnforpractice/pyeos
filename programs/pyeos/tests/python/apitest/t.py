import os
import sys
import imp
import json
import time
import wallet
import eosapi
import initeos
import eosapi
from eosapi import N

from common import prepare, producer

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('apitest', 'apitest.py', 'apitest.abi', __file__, 6)
        ret = 0
        try:
            ret =  func(*args, **kwargs)
        except Exception as e:
            s = eosapi.JsonStruct(e.args[0])
            print(s)
        return ret
    return func_wrapper

@init
def test():
    with producer:
        r = eosapi.push_action('apitest','sayhello','',{'apitest':'active'})
        assert r

@init
def inline_send():
    with producer:
        r = eosapi.push_action('apitest', 'inlinesend', '', {'apitest':'active'})
        assert r

@init
def deffer_send():
    with producer:
        r = eosapi.push_action('apitest', 'deffersend', '', {'apitest':'active', 'hello':'active'})
        assert r

@init
def call_wasm():
    with producer:
        r = eosapi.push_action('apitest', 'callwasm', '', {'apitest':'active'})
        assert r

@init
def test2(count):
    actions = []
    for i in range(count):
        action = ['apitest', 'callwasm', str(i), {'apitest':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions, True)
    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()

def change_owner_key():
    '''
    struct updateauth {
    account_name                      account;
    permission_name                   permission;
    permission_name                   parent;
    authority                         data;
    uint32_t                          delay;
   '''
    key = 'EOS8muoEdY4L9WRYXYB55WmEzYw9A314fW1uMPUqUrFUBMMjWNpxd'
    auth = eosapi.pack_updateauth('hello', 'owner', '', key, 0)
    act = [N('eosio'), N('updateauth'), [[N('hello'), N('owner')]], auth]
    r = eosapi.push_transactions2([[act]])
    assert r
    eosapi.produce_block()

def change_active_key():
    key = 'EOS8muoEdY4L9WRYXYB55WmEzYw9A314fW1uMPUqUrFUBMMjWNpxd'
    auth = eosapi.pack_updateauth('test', 'active', 'owner', key, 0)
    act = ['eosio', 'updateauth', [['test', 'active']], auth]
    r = eosapi.push_transactions([[act]])
    assert r

def set_auth():
    a = {
        "account": "hello",
        "permission": "eosio.code",
        "parent": "owner",
        "auth": {
            "threshold": 1,
            "keys": [
                {
                    "key": "EOS8muoEdY4L9WRYXYB55WmEzYw9A314fW1uMPUqUrFUBMMjWNpxd",
                    "weight": 1
                }
            ],
            "accounts": [],
            "waits": []
        }
    }
    eosapi.push_action('eosio', 'updateauth', a, {'hello':'owner'})
    
def create_multisig_account():
    #PW5KKNC8zM2KVLrb1cw4YNXZ69NLK7Fr5B35wHmsPt35tyiYkY4RR
    test_keys = {
        'EOS8mqCBdG8yiaULU3YyiD1ZhySzt64KjNb1gQiy4GdUPMEQMxVJJ': '5KZB2UCfpD3qCEUviXrgMdwDb6TDpuvuWvZKH85u9mBYK85Rqzd', 
        'EOS8mrwBdRz1zyaBiMexqJw6UybR9gNLzBC1tdLqqcfVdA2TFcgAb': '5KM6MvhsNRUtafGCghEYWXYXqWidaGTKqfsida6h5mNg5ouQUTv', 
        'EOS8mqgm5kx88XzsxsAAKH5vnq1vGNcSZDfJyar17FnTm1HvBRuXi': '5JiwrohmpRR3PjUcf6NpSLBE2QdAUTJZf1tAHYvK7iAUTWnaGgZ',
        'EOS8msM7326VykwCSdEXZrj8U3goUPYWZ84KxBTy6GajGkpQCXQNX': '5K6HPpxgq5qFkRpUcaVdZVr4hmT61EzU1AFEfkm428pEQP59neB',
    }
    all_keys = wallet.list_keys()
    for pub in test_keys:
        priv = test_keys[pub]
        if not pub in all_keys:
            wallet.import_key('mywallet', priv)
    keys = [
            [
                initeos.key1, 
                initeos.key2
            ],
            [
                'EOS8mqCBdG8yiaULU3YyiD1ZhySzt64KjNb1gQiy4GdUPMEQMxVJJ',
                'EOS8mrwBdRz1zyaBiMexqJw6UybR9gNLzBC1tdLqqcfVdA2TFcgAb'
            ],
            [
                 'EOS8mqgm5kx88XzsxsAAKH5vnq1vGNcSZDfJyar17FnTm1HvBRuXi', 
                 'EOS8msM7326VykwCSdEXZrj8U3goUPYWZ84KxBTy6GajGkpQCXQNX'
            ]
        ]
    accounts = ['test', 'test1', 'test2']
    index = 0
    for key in keys:
        a = accounts[index]
        if not eosapi.get_account(a):
            eosapi.create_account('eosio', a, key[0], key[1])
        index += 1

    eosapi.produce_block()
#"test2" must come first to satisfy the tricky verifycation in authority.hpp:validate
    per = '{"threshold" : 100, "keys" : [], "accounts" : [{"permission":{"actor":"test2","permission":"active"},"weight":50},{"permission":{"actor":"test1","permission":"active"},"weight":50}]}'
#    key = 'EOS8muoEdY4L9WRYXYB55WmEzYw9A314fW1uMPUqUrFUBMMjWNpxd'
    auth = eosapi.pack_updateauth('test', 'active', 'owner', per, 0)
    act = [N('eosio'), N('updateauth'), [[N('test'), N('active')]], auth]
    r = eosapi.push_transactions2([[act]])
    assert r
    print(r)
    eosapi.produce_block()

def gen_trx():
    act = [N('hello'), N('sayhello'), [[N('hello'), N('active')]], b'jack']
    r = eosapi.gen_transaction([act])
    print(r)

def sign_trx():
    act = [N('hello'), N('sayhello'), [[N('hello'), N('active')]], b'jack']
    r = eosapi.gen_transaction([act])
    print(r)
    r = eosapi.sign_transaction(r, '5JbDP55GXN7MLcNYKCnJtfKi9aD2HvHAdY7g8m67zFTAFkY1uBB')
    r = eosapi.JsonStruct(r)
    print(r)

def push_sign_trx():
    act = [N('hello'), N('sayhello'), [[N('hello'), N('active')]], b'jack']
    r = eosapi.gen_transaction([act])

    print(json.dumps(r, sort_keys=False, indent=4, separators=(',', ': ')))

    r = eosapi.sign_transaction(r, '5JbDP55GXN7MLcNYKCnJtfKi9aD2HvHAdY7g8m67zFTAFkY1uBB')
    eosapi.push_raw_transaction(r)
    eosapi.produce_block()
