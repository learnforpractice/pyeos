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

    with producer:
        if not eosapi.get_account('hello').permissions:
            r = eosapi.create_account('eosio', 'hello', key1, key2)
            assert r

    with producer:
        r = eosapi.set_contract('hello','../../programs/pyeos/contracts/hello/hello.py','../../programs/pyeos/contracts/hello/hello.abi', 1)
#        r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
        assert r

#eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)

def test(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('hello','sayhello',name,{'hello':'active'},rawargs=True)
        assert r

def test2(count):
    import time
    import json
    functions = []
    args = []
    per = {'hello':'active'}
    for i in range(count):
        functions.append('sayhello')
        arg = str(i)
        args.append(arg)
    ret, cost = eosapi.push_messages('hello', functions, args, per, True, rawargs=True)
    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()

