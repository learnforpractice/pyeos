import os
import sys
import imp
import json
import time
import wallet
import eosapi
import initeos
from eosapi import N

from common import prepare, producer

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('apitest', 'apitest.py', 'apitest.abi', __file__)
        return func(*args, **kwargs)
    return func_wrapper

@init
def test():
    with producer:
        r = eosapi.push_action('apitest','dbtest','',{'apitest':'active'})
        assert r

@init
def inline_send():
    with producer:
        r = eosapi.push_action('apitest', 'inlinesend', '', {'hello':'active','apitest':'active'})
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

'''
# Private key: 5KedEKNC14q9uj5uWyLBunKbenokXseQaXiE6k2AGZZhskBxdrV
# Public key: EOS5vNnn5fym9FJbTMT8AvSTY13XQsoer2HDBup33uespnDhLPipG
./cleos create account eosio acc1 EOS5vNnn5fym9FJbTMT8AvSTY13XQsoer2HDBup33uespnDhLPipG EOS5vNnn5fym9FJbTMT8AvSTY13XQsoer2HDBup33uespnDhLPipG

# Private key: 5K1wrWCTvaEoxczzceDzSHEwUL97tdWFnGQpZpjrgjxqWDkty1M
# Public key: EOS7yvrzX9B2h5UUyu765y2BHmY5NSHQxCZgf1ypuRssCdG4PXzWC
./cleos create account eosio acc2 EOS7yvrzX9B2h5UUyu765y2BHmY5NSHQxCZgf1ypuRssCdG4PXzWC EOS7yvrzX9B2h5UUyu765y2BHmY5NSHQxCZgf1ypuRssCdG4PXzWC

# Private key: 5JFzjHMtsbodCkc9rwmbWscKbVZ8zjsMHPMrAw8DWoG8r3rCmdo
# Public key: EOS8juGUoC7oQbMQ7aCmTq1659U9Di4U4sfihKeyNay99SW81qRkn
./cleos create account eosio test EOS8juGUoC7oQbMQ7aCmTq1659U9Di4U4sfihKeyNay99SW81qRkn EOS8juGUoC7oQbMQ7aCmTq1659U9Di4U4sfihKeyNay99SW81qRkn

# import private key to allow changing the active permission of test account 
./cleos wallet import 5JFzjHMtsbodCkc9rwmbWscKbVZ8zjsMHPMrAw8DWoG8r3rCmdo

# change test@active permission to be 2of2 (both acc1@active and acc2@active)
./cleos set account permission test active '{"threshold" : 100, "keys" : [], "accounts" : [{"permission":{"account":"acc2","permission":"active"},"weight":50},{"permission":{"account":"acc1","permission":"active"},"weight":50}]}' owner

# deploy currency contract
./cleos create account eosio currency EOS8juGUoC7oQbMQ7aCmTq1659U9Di4U4sfihKeyNay99SW81qRkn EOS8juGUoC7oQbMQ7aCmTq1659U9Di4U4sfihKeyNay99SW81qRkn
./cleos set contract currency ~/dev/eos/build/contracts/currency/currency.wast ~/dev/eos/build/contracts/currency/currency.abi

# send 100 tokens to test account
./cleos push message eosio.token transfer '{"from":"hello","to":"test","quantity":100, "memo":"m"}'  -p eosio.token@active

# verify that we cant transfer from test account
./cleos push message currency transfer '{"from":"test","to":"currency","quantity":5}' -S currency,test -p test@active

# generate an unsigned transaction that transfers 5 tokens from "test" to "currency"
./cleos push message -s -d -x 120 currency transfer '{"from":"test","to":"currency","quantity":5}' -S currency,test -p test@active > tx_unsigned.json
cat tx_unsigned.json
{
  "ref_block_num": 760,
  "ref_block_prefix": 293222988,
  "expiration": "2017-12-04T08:14:26",
  "scope": [
    "currency",
    "test"
  ],
  "read_scope": [],
  "messages": [{
      "code": "currency",
      "type": "transfer",
      "authorization": [{
          "account": "test",
          "permission": "active"
        }
      ],
      "data": "000000000090b1ca0000001e4d75af460500000000000000"
    }
  ],
  "signatures": []
}

# acc1@active sign
./cleos sign tx_unsigned.json -k 5KedEKNC14q9uj5uWyLBunKbenokXseQaXiE6k2AGZZhskBxdrV > tx_partial.json
cat tx_partial.json
{
  "ref_block_num": 760,
  "ref_block_prefix": 293222988,
  "expiration": "2017-12-04T08:14:26",
  "scope": [
    "currency",
    "test"
  ],
  "read_scope": [],
  "messages": [{
      "code": "currency",
      "type": "transfer",
      "authorization": [{
          "account": "test",
          "permission": "active"
        }
      ],
      "data": "000000000090b1ca0000001e4d75af460500000000000000"
    }
  ],
  "signatures": [
    "20024f73477f1a570fbebe91eec7a36a4be33ebf8cd1b8b31732cc67c99a5b880b792b8aeb0c2e00ab2a3eb4cee2639ac7cb163c994ab1772021db23e5b6df5c95"
  ]
}

# acc2@active sign
./cleos sign tx_partial.json -k 5K1wrWCTvaEoxczzceDzSHEwUL97tdWFnGQpZpjrgjxqWDkty1M > tx_signed.json
cat tx_signed.json
{
  "ref_block_num": 760,
  "ref_block_prefix": 293222988,
  "expiration": "2017-12-04T08:14:26",
  "scope": [
    "currency",
    "test"
  ],
  "read_scope": [],
  "messages": [{
      "code": "currency",
      "type": "transfer",
      "authorization": [{
          "account": "test",
          "permission": "active"
        }
      ],
      "data": "000000000090b1ca0000001e4d75af460500000000000000"
    }
  ],
  "signatures": [
    "20024f73477f1a570fbebe91eec7a36a4be33ebf8cd1b8b31732cc67c99a5b880b792b8aeb0c2e00ab2a3eb4cee2639ac7cb163c994ab1772021db23e5b6df5c95",
    "2064c65a979a9c081dc5f0a1f34d43bd4fb4245298eeb9d5fb15462b43ef6837c56f814090e697a389162cd4ac5f3d33bfe762e68d7289188e0307f23cd05ebd52"
  ]
}


# broadcast signed transaction
./cleos push transaction tx_signed.json
'''