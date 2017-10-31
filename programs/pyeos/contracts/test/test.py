import time
import wallet
import eosapi

def wait():
    num = eosapi.get_info().head_block_num
    while num == eosapi.get_info().head_block_num:  # wait for finish of create account
        time.sleep(0.2)

def init():
    psw = 'PW5Kd5tv4var9XCzvQWHZVyBMPjHEXwMjH1V19X67kixwxRpPNM4J'
    wallet.open('mywallet')
    wallet.unlock('mywallet', psw)
    
    key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
    key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
    
    if not eosapi.get_account('currency'):
        r = eosapi.create_account('inita', 'currency', key1, key2)
        assert r
        wait()
        
    if not eosapi.get_account('test'):
        r = eosapi.create_account('inita', 'test', key1, key2)
        assert r
        wait()
        
    r = eosapi.set_contract('currency','../../programs/pyeos/contracts/currency/currency.py','../../contracts/currency/currency.abi',1)
    assert r
    
    r = eosapi.set_contract('test','../../programs/pyeos/contracts/test/code.py','../../programs/pyeos/contracts/test/test.abi',1)
    assert r
    wait()

    #transfer some "money" to test account for test
    r = eosapi.push_message('currency','transfer','{"from":"currency","to":"test","amount":1000}',['currency','test'],{'currency':'active'})
    wait()

def send_message():
    r = eosapi.get_table('test','currency','account')
    print(r)
    r = eosapi.get_table('inita','currency','account')
    print(r)
#inita is require for scoping
    r = eosapi.push_message('test','testmsg','',['test','inita'],{'test':'active'},rawargs=True)
    assert r
    wait()
    
    r = eosapi.get_table('test','currency','account')
    print(r)
    r = eosapi.get_table('inita','currency','account')
    print(r)


def send_transaction():
    r = eosapi.get_table('test','currency','account')
    print(r)
    r = eosapi.get_table('inita','currency','account')
    print(r)

    r = eosapi.push_message('test','testts','',['test',],{'test':'active'},rawargs=True)
    assert r
    wait()
    
    r = eosapi.get_table('test','currency','account')
    print(r)
    r = eosapi.get_table('inita','currency','account')
    print(r)

    



    
