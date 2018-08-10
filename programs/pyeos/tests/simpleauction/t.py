import os
import time
import struct

import wallet
import eosapi
import initeos

from common import prepare, producer

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('auction1', 'simpleauction.py', 'simpleauction.abi', __file__, 6)
        return func(*args, **kwargs)
    return func_wrapper

@init
def start(beneficiary='hello', auctionEnd=None):
    if not auctionEnd:
        auctionEnd = int(time.time()+60)
    beneficiary = eosapi.N(beneficiary)
    msg = struct.pack('QQ', beneficiary, auctionEnd)
    r = eosapi.push_action('auction1','start', msg, {'auction1':'active'})
    assert r

@init
def end():
    r = eosapi.push_action('auction1','end', '', {'auction1':'active'})
    assert r

#for test only
@init
def reset(): 
    r = eosapi.push_action('auction1','reset', '', {'auction1':'active'})
    assert r

@init
def bid(price=0.1):
    msg = {"from":"eosio", "to":"auction1", "quantity":"%.4f EOS"%(price,), "memo":"m"}
    r = eosapi.push_action('eosio.token', 'transfer', msg, {'eosio':'active', 'auction1':'active'})
    assert r

