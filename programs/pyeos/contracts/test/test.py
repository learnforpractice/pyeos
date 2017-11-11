#./pyeos/pyeos --manual_gen_block --debug -i
import time
import wallet
import eosapi
import struct

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
        r = eosapi.set_contract('currency','../../programs/pyeos/contracts/currency/currency.py','../../contracts/currency/currency.abi',1)
        assert r
        r = eosapi.set_contract('test','../../programs/pyeos/contracts/test/code.py','../../programs/pyeos/contracts/test/test.abi',1)
        assert r

    #transfer some "money" to test account for test
    with producer:
        r = eosapi.push_message('currency','transfer','{"from":"currency","to":"test","amount":1000}',['currency','test'],{'currency':'active'})
        assert r

    #transfer some "money" to test account for test
    with producer:
        r = eosapi.push_message('eos','transfer',{"from":"inita","to":"test","amount":1000,"memo":"hello"},['inita','test'],{'inita':'active'})
        assert r

def deploy_wasm_code():
    with producer:
        r = eosapi.set_contract('test','../../build/programs/pyeos/contracts/test/code.wast','../../build/programs/pyeos/contracts/test/test.abi',0)

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
    args = {"from":"inita", "to":"inita", "amount":50}
    scopes = ['inita', 'eos']
    permissions = {'inita':'active'}

    with producer:
        r = eosapi.push_message('eos', 'lock', args, scopes, permissions)

    r = eosapi.get_account('inita')
    print(r)
    r = eosapi.get_account('test')
    print(r)

def unlock_eos():
    args = {"account":"test", "amount":1000}
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
        size = 1024*1025 
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