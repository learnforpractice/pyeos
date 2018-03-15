import struct
import eosapi
import wallet
from eoslib import N
'''
{
    "public": "EOS7zzybqATz1t6kWcPtyyKVf9xitXXZYA88bNt4WYrtwiEqACuKZ",
    "private": "5HvsFc23cbmWCYvqfj42tbbSb4kD9MXAeZKJ5NPAtj9gdusCcQH"
}

eosio: 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
'''

def init():
    psw = 'PW5JUPL2mFQTtRaxCYcuEKSiWpYuiECh6PKqKdUY17bYZNuZ3zA5Y'
    wallet.open('mywallet')
    wallet.unlock('mywallet',psw)
import time
def t():
    for i in range(1,1000):
#        print(i)
        if i%200 == 0:
            time.sleep(5.0)
            print(i)
        r = eosapi.push_message('currency','transfer','{"from":"currency","to":"inita","amount":%d}'%(i,),['currency','inita'],{'currency':'active'})

def test_db():
    import eostest
    eostest.start()
    eostest.create_account_manually(b'hello')
    eostest.get_account(b'hello')
    eostest.end()

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


def test_ts():
    import wallet
    from eostypes import PySignedTransaction, PyMessage
    import time
    print('hello')
    ts = PySignedTransaction()
    ts.reqire_scope(b'test')
    ts.reqire_scope(b'currency')

    data = struct.pack("QQQ", N(b'currency'), N(b'test'), 50)

    for i in range(10):
        msg = PyMessage()
        msg.init(b'currency', b'transfer', [[b'currency',b'active']], data)
        ts.add_message(msg)
    
#    print('+++++++++++++:',ts.get_messages())

#    ret = wallet.sign_transaction(ts)
#    print('sign ts return',ret)

#    r = eosapi.get_table('test','currency','account')
#    print(r)

    start = time.time()
    eosapi.push_transaction2(ts, True)
    print('cost:',time.time() - start)

    start = time.time()
    with producer:
        pass
    print('cost:',time.time() - start)

#    r = eosapi.get_table('test','currency','account')
#    print(r)

    