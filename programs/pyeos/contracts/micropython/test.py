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
        if not eosapi.get_account('micropython'):
                r = eosapi.create_account('inita', 'micropython', key1, key2)
                assert r
        if not eosapi.get_account('test'):
            if not eosapi.get_account('test'):
                r = eosapi.create_account('inita', 'test', key1, key2)
                assert r

    with producer:
        r = eosapi.set_contract('micropython','../../programs/pyeos/contracts/micropython/mp.py','../../contracts/currency/currency.abi',2)
#        r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
        assert r

#eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)


