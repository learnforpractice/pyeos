#./pyeos/pyeos --manual_gen_block --debug -i
import time
import wallet
import eosapi
from eosapi import N
#import util

import struct
import logging
print = logging.info

import initeos

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

producer = eosapi.Producer()

def init():
    with producer:
        if not eosapi.get_account('currency'):
            r = eosapi.create_account('inita', 'currency', initeos.key1, initeos.key2)

        if not eosapi.get_account('test'):
            r = eosapi.create_account('inita', 'test', initeos.key1, initeos.key2)
            assert r

    with producer:
        r = eosapi.set_contract('currency', '../../programs/pyeos/contracts/currency/currency.py', '../../contracts/currency/currency.abi', eosapi.py_vm_type)
        assert r
        r = eosapi.set_contract('test','../../programs/pyeos/contracts/test/code.py','../../programs/pyeos/contracts/test/test.abi', eosapi.py_vm_type)
        assert r

    #transfer some "money" to test account for test
    with producer:
        r = eosapi.push_message('currency','transfer','{"from":"currency","to":"test","quantity":1000}',['currency','test'],{'currency':'active'})
        assert r

    #transfer some "money" to test account for test
    with producer:
        r = eosapi.push_message('eos','transfer',{"from":"inita","to":"test","amount":1000,"memo":"hello"},['inita','test'],{'inita':'active'})
        assert r

def deploy_wasm_code():
    with producer:
        r = eosapi.set_contract('test','../../build/programs/pyeos/contracts/test/code.wast','../../build/programs/pyeos/contracts/test/test.abi',0)

def test_db():
    with producer:
        test = {'name':'test', 'balance':[1,2,3]}
        r = eosapi.push_message('test','test', test, ['test', 'eos'], {'test':'active'})
        assert r

def test_rw_db():
    with producer:
        r = eosapi.push_message('test', 'testrwdb', {'data':0}, ['test', 'eos'], {'test':'active'})
        assert r

def send_message():
    r = eosapi.get_table('test','currency','account')
    print(r)
    r = eosapi.get_table('inita','currency','account')
    print(r)
#inita is require for scoping
    with producer:
        r = eosapi.push_message('test','testmsg','',['test','inita'],{'test':'active'},rawargs=True)
        assert r
    
    r = eosapi.get_table('test','currency','account')
    print(r)
    r = eosapi.get_table('inita','currency','account')
    print(r)


def send_transaction():
    r = eosapi.get_table('test','currency','account')
    print(r)
    r = eosapi.get_table('inita','currency','account')
    print(r)

    with producer:
        r = eosapi.push_message('test','testts','',['test',],{'test':'active'},rawargs=True)
        assert r
    
    r = eosapi.get_table('test','currency','account')
    print(r)
    r = eosapi.get_table('inita','currency','account')
    print(r)
    
    producer()

    r = eosapi.get_table('test','currency','account')
    print(r)
    r = eosapi.get_table('inita','currency','account')
    print(r)


def send_eos_inline():
    args = {"from":"inita", "to":"test", "amount":1000, "memo":"hello"}
    scopes = ['test', 'inita']
    permissions = {'inita':'active'}
    
    with producer:
        r = eosapi.push_message('eos', 'transfer', args, scopes, permissions)

def lock_eos():
#    args = {"from":"inita", "to":"test", "amount":1000, "memo":"hello"}
#    scopes = ['test', 'inita', 'eos']
    args = {"from":"inita", "to":"test", "amount":50}
    scopes = ['inita', 'eos', 'test']
    permissions = {'inita':'active'}

    with producer:
        r = eosapi.push_message('eos', 'lock', args, scopes, permissions)

    r = eosapi.get_account('inita')
    print(r)
    r = eosapi.get_account('test')
    print(r)

def unlock_eos():
    args = {"account":"test", "amount":50}
    scopes = ['test', 'eos']
    permissions = {'test':'active'}

    with producer:
        r = eosapi.push_message('eos', 'unlock', args, scopes, permissions)

    r = eosapi.get_account('inita')
    print(r)
    r = eosapi.get_account('test')
    print(r)

def claim_eos():
    args = {"account":"test", "amount":1000}
    scopes = ['test', 'eos']
    permissions = {'test':'active'}

    with producer:
        r = eosapi.push_message('eos', 'claim', args, scopes, permissions)

    r = eosapi.get_account('inita')
    print(r)
    r = eosapi.get_account('test')
    print(r)

def test_memory():
    with producer:
        size = 2024*1025 
        print(size)
        #should throw an exception
        r = eosapi.push_message('test', 'testmem', {'data':size}, ['test','inita'], {'test':'active'})
        assert not r

    with producer:
        size = 1024
        print(size)
        r = eosapi.push_message('test', 'testmem', {'data':size}, ['test','inita'], {'test':'active'})
        assert r

def test_time_out():
    with producer:
        r = eosapi.push_message('test', 'testtimeout', {'data':0}, ['test','inita'], {'test':'active'})
        assert not r

def show_result():
    r = eosapi.get_account('test')

    print('r.balance:',r.balance)
    print('r.stakedBalance:',r.stakedBalance)
    print('r.unstakingBalance:',r.unstakingBalance)

    r = eosapi.get_account('currency')
    print('r.stakedBalance:',r.stakedBalance)
    print('r.unstakingBalance:',r.unstakingBalance)


def test_stake():
    with producer:
        stake = {'from':'test','to':'currency','amount':1}
        r = eosapi.push_message('eos', 'stake', stake, ['eos', 'test', 'currency'], {'test':'active'})
        assert r
    show_result()

def test_unstake():
    with producer:
        stake = {'from':'test','to':'currency','amount':1}
        r = eosapi.push_message('eos', 'unstake', stake, ['eos', 'test', 'currency'], {'test':'active'})
        assert r
    show_result()

def test_release():
    with producer:
        stake = {'from':'test','to':'currency','amount':1}
        r = eosapi.push_message('eos', 'release', stake, ['eos', 'test', 'currency'], {'test':'active'})
        assert r
    show_result()

def test_rent():
    balance_test1 = eosapi.get_account('test')
    balance_currency1 = eosapi.get_account('currency')
    amount = 1
    with producer:
        stake = {'from':'test','to':'currency','amount':amount}
        r = eosapi.push_message('eos', 'stake', stake, ['eos', 'test', 'currency'], {'test':'active'})
        assert r

    balance_test2 = eosapi.get_account('test')
    balance_currency2 = eosapi.get_account('currency')

    assert balance_test1.balance == balance_test2.balance + amount
    assert balance_currency1.balance == balance_currency2.balance
    assert balance_currency1.stakedBalance + amount == balance_currency2.stakedBalance


    balance_test1 = balance_test2
    balance_currency1 = balance_currency2

    with producer:
        stake = {'from':'test','to':'currency','amount':amount}
        r = eosapi.push_message('eos', 'unstake', stake, ['eos', 'test', 'currency'], {'test':'active'})
        assert r

    balance_test2 = eosapi.get_account('test')
    balance_currency2 = eosapi.get_account('currency')

    assert balance_test1.balance == balance_test2.balance
    assert balance_currency1.balance == balance_currency2.balance
    assert balance_currency1.stakedBalance == balance_currency2.stakedBalance + amount
    assert balance_currency1.unstakingBalance == balance_currency2.unstakingBalance - amount

    balance_test1 = balance_test2
    balance_currency1 = balance_currency2

    with producer:
        stake = {'from':'test','to':'currency','amount':amount}
        r = eosapi.push_message('eos', 'release', stake, ['eos', 'test', 'currency'], {'test':'active'})
        assert r

    balance_test2 = eosapi.get_account('test')
    balance_currency2 = eosapi.get_account('currency')

    print(balance_test1.balance, balance_test2.balance)

    assert balance_test1.balance + amount == balance_test2.balance
    assert balance_currency1.balance == balance_currency2.balance
    assert balance_currency1.stakedBalance == balance_currency2.stakedBalance
    assert balance_currency1.unstakingBalance == balance_currency2.unstakingBalance + amount

def test_rent2():
    balance_test1 = eosapi.get_account('test')
    balance_currency1 = eosapi.get_account('currency')
    amount = 1
    with producer:
        stake = {'from':'test','to':'currency','amount':amount}
        r = eosapi.push_message('eos', 'stake', stake, ['eos', 'test', 'currency'], {'test':'active'})
        assert r

    amount = 2
    with producer:
        stake = {'from':'inita','to':'currency','amount':amount}
        r = eosapi.push_message('eos', 'stake', stake, ['eos', 'inita', 'currency'], {'inita':'active'})
        assert r

    #try to unstake 1000 eos from test
    amount = 1000
    with producer:
        stake = {'from':'test','to':'currency','amount':amount}
        r = eosapi.push_message('eos', 'unstake', stake, ['eos', 'test', 'currency'], {'test':'active'})
        assert not r

def show_rent_result(from_, to_):
    balance = eosapi.get_account(from_).balance
    
    eos = N(b'eos')
    keys = struct.pack('Q', N(to_))
    values = bytes(16)
    ret = util.load(eos, eos, N(from_), keys, 0, 0, values)

    result = struct.unpack('QQ', values)
    print(ret, balance, result)

def test_rent3():
    balance_test1 = eosapi.get_account('test')
    balance_currency1 = eosapi.get_account('currency')
    amount = 1

    show_rent_result(b'test', b'currency')

    with producer:
        stake = {'from':'test','to':'currency','amount':amount}
        r = eosapi.push_message('eos', 'stake', stake, ['eos', 'test', 'currency'], {'test':'active'})
        assert r

    show_rent_result(b'test', b'currency')

    amount = 1
    #try to unstake 1000 eos from test
    with producer:
        stake = {'from':'test','to':'currency','amount':amount}
        r = eosapi.push_message('eos', 'unstake', stake, ['eos', 'test', 'currency'], {'test':'active'})
        assert r

    show_rent_result(b'test', b'currency')

    amount = 1
    with producer:
        stake = {'from':'test','to':'currency','amount':amount}
        r = eosapi.push_message('eos', 'release', stake, ['eos', 'test', 'currency'], {'test':'active'})
        assert r

    show_rent_result(b'test', b'currency')

def test_util():
    import util
    keys = struct.pack('Q', N('currency'))
    values = bytes(16)
    eos = N('eos')
    ret = util.load(eos, eos, N('test'), keys, 0, 0, values)
    print('+++++++eoslib.load return:',ret)
    print(values)
    results = struct.unpack('QQ', values)
    print(results)

def test_exec():
    with producer:
        r = eosapi.push_message('test', 'testexec', {'data':0}, ['test','inita'], {'test':'active'})
        assert not r

def test_import():
    with producer:
        r = eosapi.push_message('test', 'testimport', {'data':0}, ['test','inita'], {'test':'active'})
        assert not r

def test_load_str():
    with producer:
        r = eosapi.push_message('test', 'testloadstr', {'data':0}, ['test','inita'], {'test':'active'})
        assert r

def test_recursive():
    with producer:
        r = eosapi.push_message('test', 'testrecursive', {'data':0}, ['test','inita'], {'test':'active'})
        assert not r

def timeit(count):
    import time
    d = open('../../programs/pyeos/contracts/hello/hello.py','r').read()
    start = time.time()
    for i in range(count):
        debug.eval(d)
    cost = time.time() - start
    print(cost, cost/count, 1/(cost/count))

def timeit2(count):
    import time
    start = time.time()
    for i in range(100,100+count):
        r = eosapi.push_message('hello','sayhello',str(i), {'hello':'active'}, True, rawargs=True)
    cost = time.time() - start
    eosapi.produce_block()
    print(cost, cost/count, 1/(cost/count))

def py_timeit(count):
    import time
    
    contracts = []
    functions = []
    args = []
    per = []
    start = time.time()
    for i in range(count):
        args.append(str(i+100))
        contracts.append('hello')
        per.append({'hello':'active'})
    ret = eosapi.push_messages(contracts, functions, args, per, True, rawargs=True)
    assert ret
    cost = ret['cost_time']
    print(cost, cost/count, 1/(cost/count))
    eosapi.produce_block()


def evm_timeit(count):
    import time
    code = '6060604052341561000f57600080fd5b6b06c9144c1c690d4cb4000000600081905550600054600160003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208190555062bc614e60016000600173ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208190555060016000600173ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205460016000600273ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002081905550610c3a806101026000396000f3006060604052600436106100a4576000357c0100000000000000000000000000000000000000000000000000000000900463ffffffff16806306fdde03146100a9578063095ea7b31461013757806318160ddd1461019157806323b872dd146101ba578063313ce5671461023357806370a082311461026257806395d89b41146102af57806396178c201461033d578063a9059cbb14610366578063dd62ed3e146103c0575b600080fd5b34156100b457600080fd5b6100bc61042c565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156100fc5780820151818401526020810190506100e1565b50505050905090810190601f1680156101295780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b341561014257600080fd5b610177600480803573ffffffffffffffffffffffffffffffffffffffff16906020019091908035906020019091905050610465565b604051808215151515815260200191505060405180910390f35b341561019c57600080fd5b6101a46105ec565b6040518082815260200191505060405180910390f35b34156101c557600080fd5b610219600480803573ffffffffffffffffffffffffffffffffffffffff1690602001909190803573ffffffffffffffffffffffffffffffffffffffff169060200190919080359060200190919050506105f2565b604051808215151515815260200191505060405180910390f35b341561023e57600080fd5b6102466108ed565b604051808260ff1660ff16815260200191505060405180910390f35b341561026d57600080fd5b610299600480803573ffffffffffffffffffffffffffffffffffffffff169060200190919050506108f2565b6040518082815260200191505060405180910390f35b34156102ba57600080fd5b6102c261093b565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156103025780820151818401526020810190506102e7565b50505050905090810190601f16801561032f5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b341561034857600080fd5b610350610974565b6040518082815260200191505060405180910390f35b341561037157600080fd5b6103a6600480803573ffffffffffffffffffffffffffffffffffffffff1690602001909190803590602001909190505061097a565b604051808215151515815260200191505060405180910390f35b34156103cb57600080fd5b610416600480803573ffffffffffffffffffffffffffffffffffffffff1690602001909190803573ffffffffffffffffffffffffffffffffffffffff16906020019091905050610b50565b6040518082815260200191505060405180910390f35b6040805190810160405280600c81526020017f4d6f74696f6e20546f6b656e000000000000000000000000000000000000000081525081565b6000808214806104f157506000600360003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054145b15156104fc57600080fd5b81600360003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60005481565b600080600360008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020549050600073ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff16141515156106b057600080fd5b8083111515156106bf57600080fd5b61071183600160008873ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054610bd790919063ffffffff16565b600160008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055506107a683600160008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054610bf090919063ffffffff16565b600160008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055506107fc8382610bd790919063ffffffff16565b600360008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508373ffffffffffffffffffffffffffffffffffffffff168573ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef856040518082815260200191505060405180910390a360019150509392505050565b601281565b6000600160008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020549050919050565b6040805190810160405280600381526020017f4d544e000000000000000000000000000000000000000000000000000000000081525081565b60045481565b60008073ffffffffffffffffffffffffffffffffffffffff168373ffffffffffffffffffffffffffffffffffffffff16141515156109b757600080fd5b610a0982600160003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054610bd790919063ffffffff16565b600160003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002081905550610a9e82600160008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054610bf090919063ffffffff16565b600160008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a36001905092915050565b6000600360008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054905092915050565b6000828211151515610be557fe5b818303905092915050565b6000808284019050838110151515610c0457fe5b80915050929150505600a165627a7a72305820d2d7545eed342fcc13822a588988ee55b4466eb07980758fd484e9d593a8bf370029'
    start = time.time()
    for i in range(count):
        r = debug.evm_test('', code)
    cost = time.time() - start
    print(cost, cost/count, 1/(cost/count))

def wasm_test(count):
    import time
    
    contracts = []
    functions = []
    args = []
    per = []
    start = time.time()
    for i in range(count):
        functions.append('issue')
        arg = {"to":"currency","quantity":"1000.0000 CUR","memo":str(i)}
        args.append(str(arg).replace("'",'"'))
        contracts.append('currency')
        per.append({'currency':'active'})

    ret = eosapi.push_messages(contracts, functions, args, per, True, False)
    assert ret
    cost = ret['cost_time']
    print(cost, cost/count, 1/(cost/count))
    eosapi.produce_block()

def t():
    import time
    start = time.time()
    r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
    cost = time.time() - start
    print(cost, cost/1, 1/(cost/1))
    eosapi.produce_block()


