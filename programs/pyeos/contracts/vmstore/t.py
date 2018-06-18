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

V = 2

@init()
def deploy(d=True):
    sync = Sync('vmstore', _dir=os.path.dirname(__file__), _ignore=['t.py', 'vmstore.py'])
    
    if sys.platform == 'darwin':
        platform = '1'
    else:
        raise Exception("platform is not supported")

    if d:
        aa = [ #  name                  type     version                   path
                ['vm.wasm.'+platform+'.'+str(V),    0,         V,         "../libraries/vm_wasm/libvm_wasmd.dylib"],
                ['vm.py.'+platform+'.'+str(V),      1,         V,         "../libraries/vm_py/libvm_py-6d.dylib"],
                ['vm.eth.'+platform+'.'+str(V),     2,         V,         "../libraries/vm_eth/libvm_ethd.dylib"],
            ]
    else:
        aa = [ #  name                  type     version                   path
                ['vm.wasm.'+platform+'.'+str(V),    0,         V,         "../libraries/vm_wasm/libvm_wasm.dylib"],
                ['vm.py.'+platform+'.'+str(V),      1,         V,         "../libraries/vm_py/libvm_py-6.dylib"],
                ['vm.eth.'+platform+'.'+str(V),     2,         V,         "../libraries/vm_eth/libvm_eth.dylib"],
             ]

    for a in aa:
        debug.mp_set_max_execution_time(10000_000)
#        sync.deploy_vm(*a)
        deploy_vm(*a)

def deploy_vm(vm_name, type, version, file_name):
    account = 'vmstore' #eosapi.N('vmstore')
    print("++++++++++++++++++++deploy vm: ", vm_name)
    vm_name = eosapi.N(vm_name)

    f = open(file_name, 'rb')
    data = f.read()
    compressed = eosapi.zlib_compress_data(data)

    file_size = len(data)
    compressed_file_size = len(compressed)

    piece = 128*1024
    index = 1
    for i in range(0, len(compressed), piece):
        data = compressed[i:i+piece]
        msg = int.to_bytes(eosapi.N(account), 8, 'little') #scope
        msg += int.to_bytes(vm_name, 8, 'little') #table
        msg += int.to_bytes(index, 8, 'little') #id
        msg += data
        print(account)
        r = eosapi.push_action(account,'deploy',msg,{account:'active'})
        assert r
        index += 1

    msg = int.to_bytes(eosapi.N(account), 8, 'little') #scope
    msg += int.to_bytes(eosapi.N(account), 8, 'little') #table
    msg += int.to_bytes(vm_name, 8, 'little') #id
    msg += int.to_bytes(type, 4, 'little')
    msg += int.to_bytes(version, 4, 'little')
    msg += int.to_bytes(os.path.getsize(file_name), 4, 'little')
    msg += int.to_bytes(compressed_file_size, 4, 'little')

    print('++++++++++++++++deply:', file_name)
    r = eosapi.push_action(account,'deploy',msg,{account:'active'})
    assert r

@init()
def delete(vm='vm.py.1'):
    debug.mp_set_max_execution_time(10000_000)
    
    account = 'vmstore' #eosapi.N('vmstore')
    msg = int.to_bytes(eosapi.N(vm), 8, 'little') #scope
    r = eosapi.push_action(account,'delete',msg,{account:'active'})
    assert r

@init()
def activatevm():
    msg = int.to_bytes(N('vm.py.1'), 8, 'little') #vm name, include platform id
    msg += int.to_bytes(1, 8, 'little') #type: py
    msg += int.to_bytes(V, 8, 'little') #type: version
    act = [N('eosio'), N('activatevm'), [[N('eosio'), N('active')]], msg]
    r = eosapi.push_transactions([[act]])
    assert r[0]

@init()
def publish():
    contracts_path = os.path.join(os.getcwd(), '..', 'contracts')
    _path = os.path.join(contracts_path, 'eosio.system', 'eosio.system')
    abi = _path + '.abi'

    setabi = eosapi.pack_setabi(abi, eosapi.N('eosio'))
    assert setabi

    setabi_action = [N('eosio'), N('setabi'), [[N('eosio'), N('active')]], setabi]
    r = eosapi.push_transactions([[setabi_action]])
    assert r[0]

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

