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

    if not eosapi.get_account('currency'):
        with producer:
            r = eosapi.create_account('inita', 'currency', key1, key2)
            assert r
    with producer:
        r = eosapi.set_contract('currency','../../programs/pyeos/contracts/currency/currency.py','../../contracts/currency/currency.abi',1)
        assert r

def test():
    with producer:
        r = eosapi.push_message('currency','transfer','{"from":"currency","to":"inita","amount":1000}',['currency','inita'],{'currency':'active'})
        assert r
    r = eosapi.get_table('inita','currency','account')
    print(r)
