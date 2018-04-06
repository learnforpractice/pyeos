import time
import wallet
import eosapi
import initeos
producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init():

    if not eosapi.get_account('storage').permissions:
        with producer:
            r = eosapi.create_account('eosio', 'storage', initeos.key1, initeos.key2)
            assert r

    with producer:
        r = eosapi.set_contract('storage','../../programs/pyeos/contracts/storage/storage.py','../../programs/pyeos/contracts/storage/test.abi', 1)
        assert r

def test(name='abc'):
    with producer:
        r = eosapi.push_message('storage','storagetest', name ,{'storage':'active'},rawargs=True)
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
        contracts.append('storage')
        per.append({'storage':'active'})
        
    ret = eosapi.push_messages(contracts, functions, args, per, True, rawargs=True)
    assert ret
    cost = ret['cost_time']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()
