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
                prepare('vmstore', 'vmstore.wast', 'vmstore.abi', __file__)
            else:
                prepare('vmstore', 'vmstore.py', 'vmstore.abi', __file__)
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
    sync = Sync('vmstore', _dir=os.path.dirname(__file__), _ignore=['t.py', 'vmstore.py'])
    
    if sys.platform == 'darwin':
        platform = '1'
    else:
        raise Exception("platform is not supported")

    V = 15
    if d:
        aa = [ #  name                  type     version                   path
                ['vm.wasm.'+platform,    0,         V,         "../libraries/vm_wasm/libvm_wasmd.dylib"],
                ['vm.py.'+platform,      1,         V,         "../libraries/vm_py/libvm_py-1d.dylib"],
                ['vm.eth.'+platform,     2,         V,         "../libraries/vm_eth/libvm_ethd.dylib"],
            ]
    else:
        aa = [ #  name                  type     version                   path
                ['vm.wasm.'+platform,    0,         V,         "../libraries/vm_wasm/libvm_wasm.dylib"],
                ['vm.py.'+platform,      1,         V,         "../libraries/vm_py/libvm_py-1.dylib"],
                ['vm.eth.'+platform,     2,         V,         "../libraries/vm_eth/libvm_eth.dylib"],
             ]

    for a in aa:
        debug.mp_set_max_execution_time(10000_000)
        if d:
            sync.deploy_vm(*a)
        else:
            sync.deploy_vm(*a)

@init()
def test2(count=100):
    actions = []
    for i in range(count):
#        action = ['eosio.token', 'issue', {'eosio':'active'}, {"to":"eosio","quantity":"0.0100 EOS","memo":""}]
#        actions.append(action)

#        msg = {"issuer":"eosio","maximum_supply":"10000000000.0000 EOS"}
#        action = ['eosio.token', 'create', {'eosio.token':'active'}, msg]

        msg = {"from":"eosio", "to":"hello", "quantity":"0.0001 EOS", "memo":"m"}
        action = ['eosio.token', 'transfer', {'eosio':'active'}, msg]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions, True)
    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

@init()
def test3():
    msg = {"issuer":"eosio","maximum_supply":"10000000000.0000 EOS"}
    r = eosapi.push_action('eosio.token', 'create', msg, {'eosio.token':'active'})
    print(r)
    assert r

