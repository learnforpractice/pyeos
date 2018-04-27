import os
import sys
import imp
import time
import wallet
import eosapi
import initeos

from common import smart_call, producer

def init(func):
    def func_wrapper(*args):
        return smart_call('apitest', 'apitest.py', 'apitest.abi', 2, __file__, func, __name__, args)
    return func_wrapper

@init
def test():
    with producer:
        r = eosapi.push_message('apitest','dbtest','',{'apitest':'active', 'test2':'active'},rawargs=True)
        assert r

@init
def inline_send():
    with producer:
        r = eosapi.push_message('apitest', 'inlinesend', '', {'hello':'active'}, rawargs=True)
        assert r

@init
def deffer_send():
    with producer:
        r = eosapi.push_message('apitest', 'deffersend', '', {'apitest':'active', 'hello':'active'}, rawargs=True)
        assert r
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