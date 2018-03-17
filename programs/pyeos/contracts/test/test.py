#./pyeos/pyeos --manual_gen_block --debug -i
import time
import wallet
import eosapi
from eosapi import N
#import util

import struct
import logging
print = logging.info

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

producer = eosapi.Producer()

def init():
    psw = 'PW5Kd5tv4var9XCzvQWHZVyBMPjHEXwMjH1V19X67kixwxRpPNM4J'
    wallet.open('mywallet')
    wallet.unlock('mywallet', psw)
    
    key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
    key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
    
    with producer:
        if not eosapi.get_account('currency'):
            r = eosapi.create_account('inita', 'currency', key1, key2)

        if not eosapi.get_account('test'):
            r = eosapi.create_account('inita', 'test', key1, key2)
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



