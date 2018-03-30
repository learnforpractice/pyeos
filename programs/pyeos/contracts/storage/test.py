import time
import wallet
import eosapi

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init():
    psw = 'PW5KQ5RieLowfi5gX81bQ4zTnHde3ZENpooryxdm4xbdoH6bNW9SZ'
    wallet.open('mywallet')
    wallet.unlock('mywallet', psw)
    
    key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
    key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'


    if not eosapi.get_account('storage').permissions:
        with producer:
            r = eosapi.create_account('eosio', 'storage', key1, key2)
            assert r

    with producer:
        r = eosapi.set_contract('storage','../../programs/pyeos/contracts/storage/storage.py','../../programs/pyeos/contracts/storage/test.abi', 1)
        assert r

def test():
    with producer:
        r = eosapi.push_message('storage','storagetest','',{'storage':'active'},rawargs=True)
        assert r

