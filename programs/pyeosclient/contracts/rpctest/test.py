import time
import wallet
import eosapi
import initeos

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init():
    with producer:
        if not eosapi.get_account('rpctest').permissions:
            r = eosapi.create_account('eosio', 'rpctest', initeos.key1, initeos.key2)
            assert r

    with producer:
        r = eosapi.set_contract('rpctest','../../programs/pyeos/contracts/rpctest/rpctest.py','../../programs/pyeos/contracts/rpctest/rpctest.abi', 3)
#        r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
        assert r

def test(name=None):
    with producer:
        r = eosapi.push_message('rpctest','sayhello', 'hello,world',{'rpctest':'active'},rawargs=True)
        assert r

def test2(count):
    import time
    import json
    
    contracts = []
    functions = []
    args = []
    per = []
    for i in range(count):
        functions.append('sayhello')
        arg = str(i)
        args.append(arg)
        contracts.append('rpctest')
        per.append({'rpctest':'active'})
    ret = eosapi.push_messages(contracts, functions, args, per, True, rawargs=True)
    assert ret
    cost = ret['cost_time']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()


