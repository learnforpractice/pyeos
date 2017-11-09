import time
import wallet
import eosapi

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init():
    psw = 'PW5Kd5tv4var9XCzvQWHZVyBMPjHEXwMjH1V19X67kixwxRpPNM4J'
    wallet.open('mywallet')
    wallet.unlock('mywallet', psw)
    
    key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
    key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'

    with producer:
        if not eosapi.get_account('currency'):
                r = eosapi.create_account('inita', 'currency', key1, key2)
                assert r
        if not eosapi.get_account('test'):
                if not eosapi.get_account('test'):
                    r = eosapi.create_account('inita', 'test', key1, key2)
                    assert r

    with producer:
        r = eosapi.set_contract('currency','../../programs/pyeos/contracts/currency/currency.py','../../contracts/currency/currency.abi',1)
        assert r

#eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)


def test():
    with producer:
        r = eosapi.push_message('currency','transfer','{"from":"currency","to":"inita","amount":1000}',['currency','inita'],{'currency':'active'})
        assert r
    r = eosapi.get_table('inita','currency','account')
    print(r)

def test_performance():
    import wallet
    import struct
    from eoslib import N
    from eostypes import PySignedTransaction, PyMessage
    import time
    print('hello')
    
    ts = PySignedTransaction()
    ts.reqire_scope(b'test')
    ts.reqire_scope(b'currency')

    data = struct.pack("QQQ", N(b'currency'), N(b'test'), 1)

    for i in range(900):
        msg = PyMessage()
        msg.init(b'currency', b'transfer', [[b'currency',b'active']], data)
        ts.add_message(msg)

    start = time.time()
    eosapi.push_transaction2(ts, True)
    print('cost:',time.time() - start)

    start = time.time()
    with producer:
        pass
    print('cost:',time.time() - start)

def compare_performance():
    with producer:
        r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
        assert r

    time.sleep(3.0)
    test_performance()

    info = eosapi.get_code('currency')
    with producer:
        r = eosapi.set_contract('currency','../../programs/pyeos/contracts/currency/currency.py','../../contracts/currency/currency.abi',1)
        assert r

    time.sleep(3.0)
    test_performance()


#    r = eosapi.get_table('test','currency','account')
#    print(r)

