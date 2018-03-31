import time
import wallet
import eosapi
import initeos

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init():
    with producer:
        if not eosapi.get_account('kitties').permissions:
            r = eosapi.create_account('eosio', 'kitties', initeos.key1, initeos.key2)
            assert r

    with producer:
        r = eosapi.set_contract('kitties','../../programs/pyeos/contracts/cryptokitties/cryptokitties.py','../../programs/pyeos/contracts/cryptokitties/cryptokitties.abi', 1)
#        r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
        assert r

#eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)

def test(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('kitties','sayhello',name,{'kitties':'active'},rawargs=True)
        assert r

def test2(count):
    import time
    import json
    functions = []
    args = []
    per = {'kitties':'active'}
    for i in range(count):
        functions.append('sayhello')
        arg = str(i)
        args.append(arg)
    ret, cost = eosapi.push_messages('kitties', functions, args, per, True, rawargs=True)
    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()

