import os
import sys
import time
import struct

import debug
import wallet
import eosapi
import initeos

from eosapi import N
from tools import cpp2wast

from common import prepare, Sync, producer

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init(wasm=False):
    def init_decorator(func):
        def func_wrapper(*args, **kwargs):
            if wasm:
                prepare('native', 'native.wast', 'native.abi', __file__)
            else:
                prepare('native', 'native.py', 'native.abi', __file__)
            return func(*args, **kwargs)
        return func_wrapper
    return init_decorator

@init()
def test(msg='hello,world'):
    with producer:
        msg = {"from":"eosio", "to":"hello", "quantity":"0.0001 EOS", "memo":"m"}
        r = eosapi.push_action('eosio.token', 'transfer', msg, {'eosio':'active'})
#        print(r)
        assert r

@init()
def deploy(d=True):
    if sys.platform == 'darwin':
        PLAT = '.1'
    else:
        raise Exception("platform is not supported")

    sync = Sync('native', _dir=os.path.dirname(__file__), _ignore=['t.py', 'native.py'])
    #         name            version        path                 native lib name
    V = 3
    aa = [
          ['eosio.bios'+PLAT,    V,       'eosio.bios',       'eosio_bios_native'],
          ['eosio.msig'+PLAT,    V,       'eosio.msig',       'eosio_msig_native'],
          ['eosio.token'+PLAT,   V,       'eosio.token',      'eosio_token_native'],
          ['eosio'+PLAT,         V,       'eosio.system',     'eosio_system_native'],
          ['exchange'+PLAT,      V,       'exchange',         'exchange_native'] 
        ]

    debug.mp_set_max_execution_time(1000_000)

    for a in aa:
        if d:
            sync.deploy_native(a[0], a[1], '../../build-debug/contracts/{0}/lib{1}d.dylib'.format(a[2], a[3]))
        else:
            sync.deploy_native(a[0], a[1], '../../build/contracts/{0}/lib{1}.dylib'.format(a[2], a[3]))

@init()
def test2(count=100):
    actions = []
    for i in range(count):
#        action = ['eosio.token', 'issue', {'eosio':'active'}, {"to":"eosio","quantity":"0.0100 EOS","memo":""}]
#        actions.append(action)

#        msg = {"issuer":"eosio","maximum_supply":"10000000000.0000 EOS"}
#        action = ['eosio.token', 'create', {'eosio.token':'active'}, msg]

#hello is a micropython smart contract and transfer will call hello, so do no use hello as destination account.
        msg = {"from":"eosio", "to":"eosio.token", "quantity":"0.0001 EOS", "memo":"m"}
        action = ['eosio.token', 'transfer', msg, {'eosio':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions, True)
    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

@init()
def test3(count=100):
    actions = []

    for i in range(count):
        msg = {"from":"eosio", "to":"eosio.token", "quantity":"0.0001 EOS", "memo":str(i)}
        msg = eosapi.pack_args('eosio.token', 'transfer', msg)
        act = [N('eosio.token'), N('transfer'), [[N('eosio'), N('active')]], msg]
        actions.append([act])
    r, cost = eosapi.push_transactions(actions, True)
    print('total cost time:%.3f s, cost per TS: %.3f ms, TS per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

@init()
def test4():
    with producer:
        msg = {"issuer":"eosio","maximum_supply":"10000000000.0000 EOS"}
        r = eosapi.push_action('eosio.token', 'create', msg, {'eosio.token':'active'})
        print(r)
        assert r

