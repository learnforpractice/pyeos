import os
import json
import web3
import math

from web3 import Web3, HTTPProvider, TestRPCProvider, EthereumTesterProvider
from solc import compile_source
from web3.contract import ConciseContract

import rodb
import eosapi
import wallet
import initeos
from common import prepare
from eosapi import N

from eth_utils import (
    to_dict,
)

def compile(contract_source_code, main_class):

    compiled_sol = compile_source(contract_source_code) # Compiled source code
#    print(compiled_sol)
    
#    s = json.dumps(compiled_sol[main_class], sort_keys=False, indent=4, separators=(',', ': '))
    for _class in compiled_sol.keys():
        print(_class)

    contract_interface = compiled_sol[main_class]
    return contract_interface

contract_abi = None
def setUp():
    global contract_abi
    main_class = '<stdin>:Tester'
    greeter = os.path.join(os.path.dirname(__file__), 'tester.sol')
    with open(greeter, 'r') as f:
        contract_source_code = f.read()

    contract_interface = compile(contract_source_code, main_class)
    contract_abi = contract_interface['abi']
    bin = contract_interface['bin']
#        print(bin)

    account = 'evm'
    actions = []
    
    last_update = eosapi.get_code_update_time_ms('evm')
    modify_time = os.path.getmtime(greeter)*1000
    if last_update >= modify_time:
        return

    _src_dir = os.path.dirname(__file__)
    abi_file = os.path.join(_src_dir, 'evm.abi')
    setabi = eosapi.pack_setabi(abi_file, eosapi.N(account))
    act = ['eosio', 'setabi', setabi, {account:'active'}]
    actions.append(act)

    args = eosapi.pack_args("eosio", 'setcode', {'account':account,'vmtype':2, 'vmversion':0, 'code':bin})
    act = ['eosio', 'setcode', args, {'evm':'active'}]
    actions.append(act)

    r, cost = eosapi.push_actions(actions)
    print(r['except'])
    print(r['elapsed'])
#        call_contract(contract_interface)

def init(func):
    def func_wrapper(*args, **kwargs):
        if not eosapi.get_account('evm'):
            print('evm account not exist, create it.')
            r = eosapi.create_account('eosio', 'evm', initeos.key1, initeos.key2)
            assert r
        setUp()
        func(*args, **kwargs)
    return func_wrapper

def generate_call_params(func_name, args=(), kwargs={}):
    fn_identifier = func_name
    for abi in contract_abi:
        if 'name' in abi and abi['name'] == fn_identifier:
            fn_abi = abi
            break

    data = web3.utils.contracts.encode_transaction_data(
            web3,
            fn_identifier,
            contract_abi,
            fn_abi,
            args,
            kwargs)
    return data[2:]

@init
def test_set_value(v=119000):
    data = generate_call_params('testSetValue', (v,))
    print(data)
    args = {'from':'eosio', 'to':'evm', 'amount':0, 'data':data}
#    args = eosapi.pack_args('evm', 'transfer', args)
#    print(args)
    r = eosapi.push_action('evm', 'ethtransfer', args, {'eosio':'active'})
    print(r['except'])
    print(r['elapsed'])
    n = eosapi.N('evm')

    itr = rodb.find_i256(n, n, n, 0)
    assert itr >= 0
    value = rodb.get_i256(itr)
    assert value == v

@init
def test_get_value():
    data = generate_call_params('testGetValue', ())
    print(data)
    args = {'from':'eosio', 'to':'evm', 'amount':0, 'data':data}
    r = eosapi.push_action('evm', 'ethtransfer', args, {'eosio':'active'})
    print(r['except'])
    print(r['elapsed'])

@init
def test_transfer():

    data =generate_call_params('testTransfer')
    print(data)
    balance = eosapi.get_balance('evm')
    args = {'from':'eosio', 'to':'evm', 'amount':0, 'data':data}
    r = eosapi.push_action('evm', 'ethtransfer', args, {'eosio':'active'})
    print(r['elapsed'])
    assert math.isclose(balance, eosapi.get_balance('evm')++0.0001)

@init
def test_transfer2():
    data =generate_call_params('testTransfer')
    print(data)
    balance = eosapi.get_balance('evm')
    ret = eosapi.transfer('eosio', 'evm', 0.0001, data)
    assert ret
    assert math.isclose(balance, eosapi.get_balance('evm'))

@init
def test_memory():
    data =generate_call_params('testMemory')
    print(data)
    args = {'from':'eosio', 'to':'evm', 'amount':0, 'data':data}
    r = eosapi.push_action('evm', 'ethtransfer', args, {'eosio':'active'})
    print(r['elapsed'])

@init
def test_memory2():
    data =generate_call_params('testMemory2')
    print(data)
    args = {'from':'eosio', 'to':'evm', 'amount':0, 'data':data}
    r = eosapi.push_action('evm', 'ethtransfer', args, {'eosio':'active'})
    print(r['elapsed'])

@init
def test2(count=200):
    data =generate_call_params('testSetValue', (10,))
    args = {'from':'eosio', 'to':'evm', 'amount':0, 'data':data}
    actions = []
    for i in range(count):
        action = ['evm', 'ethtransfer', args, {'eosio':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions)
    cost = ret['elapsed']
    print(ret['except'])
    assert ret and not ret['except']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

@init
def test3(count=200):
    transactions = []
    for i in range(count):
        data =generate_call_params('testSetValue', (i,))
        args = {'from':'eosio', 'to':'evm', 'amount':0, 'data':data}
        args = eosapi.pack_args('evm', 'ethtransfer', args)
        action = ['evm', 'ethtransfer', args, {'eosio':'active'}]
        transactions.append([action,])
    ret, cost = eosapi.push_transactions(transactions)
    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

import unittest

class EVMTestCase(unittest.TestCase):
    def setUp(self):
        pass

    def test_set_value(self):
        test_set_value(100)

    def test_get_value(self):
        test_get_value()

    def test_transfer(self):
        test_transfer()
    
    def test_transfer2(self):
        test_transfer2()

    def test_memory(self):
        test_memory()
#        self.assertEqual(True, True, 'incorrect default size')

    @unittest.expectedFailure
    def test_memory_out(self):
        test_memory2()

    def tearDown(self):
        pass

                                     
def ut():
    unittest.main(module=ut.__module__, exit=False)

#@unittest.expectedFailure

def suite():
    suite = unittest.TestSuite()
    testcase = unittest.FunctionTestCase(test_transfer)
    suite.addTest(testcase)
    testcase = unittest.FunctionTestCase(test_memory)
    suite.addTest(testcase)
    testcase = unittest.FunctionTestCase(test_memory2)
    suite.addTest(testcase)

    return suite

def ut2():
    runner = unittest.TextTestRunner()
    runner.run(suite())
