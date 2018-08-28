import os
import json
import web3
import math

from web3 import Web3, HTTPProvider, TestRPCProvider, EthereumTesterProvider
from solc import compile_source
from web3.contract import ConciseContract
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


@init
def setValue(v=119000):
    fn_identifier = 'setValue'
    for abi in contract_abi:
        if 'name' in abi and abi['name'] == fn_identifier:
            fn_abi = abi
            break
    args = (v,)
    kwargs = {}

    data = web3.utils.contracts.encode_transaction_data(
            web3,
            fn_identifier,
            contract_abi,
            fn_abi,
            args,
            kwargs)
    print(data)
    data = data[2:]
    args = {'from':'eosio', 'to':'evm', 'amount':123, 'data':data}
#    args = eosapi.pack_args('evm', 'transfer', args)
#    print(args)
    r = eosapi.push_action('evm', 'ethtransfer', args, {'eosio':'active'})
    print(r['except'])
    print(r['elapsed'])

@init
def test_transfer():

    fn_identifier = 'testTransfer'

    for abi in contract_abi:
        if 'name' in abi and abi['name'] == fn_identifier:
            fn_abi = abi
            break
    args = ()
    kwargs = {}

    data = web3.utils.contracts.encode_transaction_data(
            web3,
            fn_identifier,
            contract_abi,
            fn_abi,
            args,
            kwargs)
    print(data)
    data = data[2:]
    '''
    ret = eosapi.transfer('eosio', 'evm', 0.2, data)
    assert ret
    '''
    balance = eosapi.get_balance('evm')
    args = {'from':'eosio', 'to':'evm', 'amount':0, 'data':data}
    r = eosapi.push_action('evm', 'ethtransfer', args, {'eosio':'active'})
    print(r['elapsed'])
    assert math.isclose(balance, eosapi.get_balance('evm')++0.0001)

@init
def test_transfer2():

    fn_identifier = 'testTransfer'

    for abi in contract_abi:
        if 'name' in abi and abi['name'] == fn_identifier:
            fn_abi = abi
            break
    args = ()
    kwargs = {}

    data = web3.utils.contracts.encode_transaction_data(
            web3,
            fn_identifier,
            contract_abi,
            fn_abi,
            args,
            kwargs)
    print(data)
    data = data[2:]

    balance = eosapi.get_balance('evm')
    ret = eosapi.transfer('eosio', 'evm', 0.0001, data)
    assert ret
    assert math.isclose(balance, eosapi.get_balance('evm'))

@init
def test_memory():

    fn_identifier = 'testMemory'

    for abi in contract_abi:
        if 'name' in abi and abi['name'] == fn_identifier:
            fn_abi = abi
            break
    args = ()
    kwargs = {}

    data = web3.utils.contracts.encode_transaction_data(
            web3,
            fn_identifier,
            contract_abi,
            fn_abi,
            args,
            kwargs)
    print(data)
    data = data[2:]
    '''
    ret = eosapi.transfer('eosio', 'evm', 0.2, data)
    assert ret
    '''

    args = {'from':'eosio', 'to':'evm', 'amount':0, 'data':data}
    r = eosapi.push_action('evm', 'ethtransfer', args, {'eosio':'active'})
    print(r['elapsed'])

@init
def test_memory2():

    fn_identifier = 'testMemory2'

    for abi in contract_abi:
        if 'name' in abi and abi['name'] == fn_identifier:
            fn_abi = abi
            break
    args = ()
    kwargs = {}

    data = web3.utils.contracts.encode_transaction_data(
            web3,
            fn_identifier,
            contract_abi,
            fn_abi,
            args,
            kwargs)
    print(data)
    data = data[2:]
    '''
    ret = eosapi.transfer('eosio', 'evm', 0.2, data)
    assert ret
    '''
    try:
        args = {'from':'eosio', 'to':'evm', 'amount':0, 'data':data}
        r = eosapi.push_action('evm', 'ethtransfer', args, {'eosio':'active'})
        print(r['elapsed'])
    except Exception as e:
        unittest.assertEqual(1, 0, "broken")

@init
def test2(count=200):
    fn_identifier = 'setValue'

    for abi in contract_abi:
        if 'name' in abi and abi['name'] == fn_identifier:
            fn_abi = abi
            break
    args = (1,)
    kwargs = {}
    print('+++fn_abi', fn_abi)
    data = web3.utils.contracts.encode_transaction_data(
            web3,
            fn_identifier,
            contract_abi,
            fn_abi,
            args,
            kwargs)
    print(data)
    data = data[2:]
    args = {'from':'eosio', 'to':'evm', 'amount':10, 'data':data}

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
    fn_identifier = 'setValue'

    for abi in contract_abi:
        if 'name' in abi and abi['name'] == fn_identifier:
            fn_abi = abi
            break
    args = (100,)
    kwargs = {}

    data = web3.utils.contracts.encode_transaction_data(
            web3,
            fn_identifier,
            contract_abi,
            fn_abi,
            args,
            kwargs)
    print(data)
    data = data[2:]
    transactions = []
    for i in range(count):
        args = {'from':'eosio', 'to':'evm', 'amount':i, 'data':data}
        args = eosapi.pack_args('evm', 'ethtransfer', args)
        action = ['evm', 'ethtransfer', args, {'eosio':'active'}]
        transactions.append([action,])
    ret, cost = eosapi.push_transactions(transactions)
    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

@init
def test4():
    main_class = '<stdin>:KittyCore'
    with open('../../programs/pyeos/contracts/evm/cryptokitties.sol', 'r') as f:
        contract_source_code = f.read()
        contract_interface = compile(contract_source_code, main_class)
        deploy(contract_interface)
        kitties_test(contract_interface)

import unittest

class EVMTestCase(unittest.TestCase):
    def setUp(self):
        pass

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
