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

def compile(src_file, main_class):
    abs_src_file = os.path.join(os.path.dirname(__file__), src_file)
    with open(abs_src_file, 'r') as f:
        contract_source_code = f.read()

    compiled_sol = compile_source(contract_source_code) # Compiled source code
#    print(compiled_sol)
    
#    s = json.dumps(compiled_sol[main_class], sort_keys=False, indent=4, separators=(',', ': '))
    for _class in compiled_sol.keys():
        print(_class)
    contract_interface = compiled_sol[main_class]
    return contract_interface['abi'], contract_interface['bin']

def deploy(account, bin):

    actions = []
    _src_dir = os.path.dirname(__file__)
    abi_file = os.path.join(_src_dir, 'evm.abi')
    setabi = eosapi.pack_setabi(abi_file, eosapi.N(account))
    act = ['eosio', 'setabi', setabi, {account:'active'}]
    actions.append(act)

    args = eosapi.pack_args("eosio", 'setcode', {'account':account,'vmtype':2, 'vmversion':0, 'code':bin})
    act = ['eosio', 'setcode', args, {account:'active'}]
    actions.append(act)

    r, cost = eosapi.push_actions(actions)
    print(r['except'])
    print(r['elapsed'])

contract_abi, contract_bin = compile('tester.sol', '<stdin>:Tester')

def setUp(account='evm', main_class='<stdin>:Tester', src_file='tester.sol'):
    global contract_abi
    if not eosapi.get_account(account):
        print('evm account not exist, create it.')
        r = eosapi.create_account2('eosio', account, initeos.key1, initeos.key2)
        assert r

    abs_src_file = os.path.join(os.path.dirname(__file__), src_file)
    last_update = eosapi.get_code_update_time_ms(account)
    modify_time = os.path.getmtime(abs_src_file)*1000
    if last_update >= modify_time:
        return
#        print(bin)
    contract_abi, bin = compile(src_file, main_class)
    deploy(account, bin)


def init(func):
    global contract_abi
    def func_wrapper(*args, **kwargs):
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

def call(func_name, args = (), amount=0):
    print(func_name, args)
    data =generate_call_params(func_name, args)
    print('+++call parameters:', data)
    args = {'from':'eosio', 'to':'evm', 'amount':amount, 'data':data}
    r = eosapi.push_action('evm', 'transfer', args, {'eosio':'active'})
    print(r['elapsed'])

@init
def test_set_value(v=119000):
    call('testSetValue', (v,))
    n = eosapi.N('evm')

    itr = rodb.find_i256(n, n, n, 0)
    assert itr >= 0
    value = rodb.get_i256(itr)
    print(value)
    assert value == v

@init
def test_get_value():
    call('testGetValue', ())

@init
def test_delete_value():
    n = eosapi.N('evm')
    itr = rodb.find_i256(n, n, n, 0)
    assert itr >= 0
    call('testDeleteValue', ())

    itr = rodb.find_i256(n, n, n, 0)
    assert itr < 0

@init
def test_transfer():
    balance = eosapi.get_balance('evm')
    call('testTransfer', (), amount=10000) #1.0 EOS
    assert math.isclose(balance+1, eosapi.get_balance('evm')+0.0001)

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
    call('testMemory', ())

@init
def test_memory2():
    call('testMemory2', ())

@init
def test_call():
    src_file = 'callee.sol'
    main_class = '<stdin>:Callee'
    account = 'callee'
    if not eosapi.get_account(account):
        print('account not exist, create it.')
        r = eosapi.create_account2('eosio', account, initeos.key1, initeos.key2)
        assert r

    abs_src_file = os.path.join(os.path.dirname(__file__), src_file)
    last_update = eosapi.get_code_update_time_ms(account)
    modify_time = os.path.getmtime(abs_src_file)*1000
    if last_update < modify_time:
        contract_abi, bin = compile(src_file, main_class)
        deploy(account, bin)
    call('testCall', ('0x00000000000000000000000041a3152800000000', 120))
    n = eosapi.N(account)
    itr = rodb.find_i256(n, n, n, 0)
    assert itr >= 0
    assert rodb.get_i256(itr) == 120

@init
def test_suicide():
    call('testSuicide')

@init
def test2(count=200):
    data =generate_call_params('testSetValue', (10,))
    args = {'from':'eosio', 'to':'evm', 'amount':0, 'data':data}
    actions = []
    for i in range(count):
        action = ['evm', 'transfer', args, {'eosio':'active'}]
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
        args = eosapi.pack_args('evm', 'transfer', args)
        action = ['evm', 'transfer', args, {'eosio':'active'}]
        transactions.append([action,])
    ret, cost = eosapi.push_transactions(transactions)
    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

import unittest

class EVMTestCase(unittest.TestCase):
    def setUp(self):
        pass

    def test_set_value(self):
        test_set_value()

    def test_get_value(self):
        test_get_value()

    def test_delete_value(self):
        test_delete_value()


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

    def test_call(self):
        test_call()

    def test_suicide(self):
        test_suicide()

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
