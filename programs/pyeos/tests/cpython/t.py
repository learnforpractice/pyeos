import time
import wallet
import eosapi
import initeos
import rodb as db

from eosapi import N

from common import prepare

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('counter', 'hello.py', 'hello.abi', __file__)
        return func(*args, **kwargs)
    return func_wrapper

@init
def test(name=None):
    code = N('counter')
    counter_id = N('counter')
    counter_begin = 0
    itr = db.find_i64(code, code, code, counter_id)
    if itr >= 0:
        counter_begin = db.get_i64(itr)
        counter_begin = int.from_bytes(counter_begin, 'little')

    print('counter begin: ', counter_begin)

    r = eosapi.push_action('counter', 'count', '', {'counter':'active'})
    assert r and not r['except'] 

    counter_end = 0
    itr = db.find_i64(code, code, code, counter_id)
    if itr >= 0:
        counter_end = db.get_i64(itr)
        counter_end = int.from_bytes(counter_end, 'little')

    print('counter end: ', counter_end)

    assert counter_begin + 1 == counter_end

@init
def test2(count=100):
    import time
    import json

    code = N('counter')
    counter_id = N('counter')
    counter_begin = 0
    itr = db.find_i64(code, code, code, counter_id)
    if itr >= 0:
        counter_begin = db.get_i64(itr)
        counter_begin = int.from_bytes(counter_begin, 'little')

    print('counter begin: ', counter_begin)


    actions = []
    for i in range(count):
        action = ['counter', 'count', str(i), {'counter':'active'}]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions)
    assert ret
    print(ret['elapsed'])
    print(cost)
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

    counter_end = 0
    itr = db.find_i64(code, code, code, counter_id)
    if itr >= 0:
        counter_end = db.get_i64(itr)
        counter_end = int.from_bytes(counter_end, 'little')
    print('counter end: ', counter_end)
    assert counter_begin + count == counter_end

@init
def test3(count=200):
    actions = []
    for i in range(count):
        action = ['counter', 'count', str(i), [['counter','active']]]
        actions.append([action])

    ret, cost = eosapi.push_transactions(actions)

    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, transaction per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))


def test4(count):
    start = time.time()
    for i in range(count):
        n = eosapi.N('hello')
        n = eosapi.N('hello')
        n**100
    end = time.time()
    print(1/((end-start)/count))

