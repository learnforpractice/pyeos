import time
import wallet
import eosapi
import initeos
import database_api

from eosapi import N

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init(mpy=True):
    with producer:
        if not eosapi.get_account('counter').permissions:
            r = eosapi.create_account('eosio', 'counter', initeos.key1, initeos.key2)
            assert r

    with producer:
        if mpy:
            with open('../../programs/pyeos/contracts/counter/counter.mpy', 'wb') as f:
                f.write(eosapi.mp_compile('../../programs/pyeos/contracts/counter/counter.py'))
            r = eosapi.set_contract('counter','../../programs/pyeos/contracts/counter/counter.mpy','../../programs/pyeos/contracts/counter/counter.abi', 1)
        else:
            r = eosapi.set_contract('counter','../../programs/pyeos/contracts/counter/counter.py','../../programs/pyeos/contracts/counter/counter.abi', 1)
#        r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
            assert r

#eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)

def test(name=None):
    with producer:
        if not eosapi.get_account('counter').permissions:
            r = eosapi.create_account('eosio', 'counter', initeos.key1, initeos.key2)
            assert r

    code = N('counter')
    counter_id = N('counter')
    counter_begin = 0
    itr = database_api.find_i64(code, code, code, counter_id)
    if itr >= 0:
        counter_begin = database_api.get_i64(itr)
        counter_begin = int.from_bytes(counter_begin, 'little')

    print('counter begin: ', counter_begin)

    with producer:
        r = eosapi.push_message('counter', 'count', '', {'counter':'active'}, rawargs=True)
        assert r


    counter_end = 0
    itr = database_api.find_i64(code, code, code, counter_id)
    if itr >= 0:
        counter_end = database_api.get_i64(itr)
        counter_end = int.from_bytes(counter_end, 'little')

    print('counter end: ', counter_end)

    assert counter_begin + 1 == counter_end

def test2(count):
    import time
    import json

    code = N('counter')
    counter_id = N('counter')
    counter_begin = 0
    itr = database_api.find_i64(code, code, code, counter_id)
    if itr >= 0:
        counter_begin = database_api.get_i64(itr)
        counter_begin = int.from_bytes(counter_begin, 'little')

    print('counter begin: ', counter_begin)

    contracts = []
    functions = []
    args = []
    per = []
    for i in range(count):
        functions.append('count')
        arg = str(i)
        args.append(arg)
        contracts.append('counter')
        per.append({'counter':'active'})
    ret = eosapi.push_messages(contracts, functions, args, per, True, rawargs=True)
    assert ret
    cost = ret['cost_time']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()

    counter_end = 0
    itr = database_api.find_i64(code, code, code, counter_id)
    if itr >= 0:
        counter_end = database_api.get_i64(itr)
        counter_end = int.from_bytes(counter_end, 'little')
    print('counter end: ', counter_end)
    assert counter_begin + count == counter_end

