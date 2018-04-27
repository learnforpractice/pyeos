import time
import wallet
import eosapi
import initeos

from common import smart_call, producer

def init(func):
    def func_wrapper(*args):
        return smart_call('bugs', 'bugs.py', 'bugs.abi', 2, __file__, func, __name__, args)
    return func_wrapper

@init
def t():
    with producer:
        r = eosapi.push_message('bugs','t1','',{'bugs':'active'},rawargs=True)
        assert r
    eosapi.produce_block()

#test deeply recursive generators
@init
def t2():
    with producer:
        r = eosapi.push_message('bugs','t2','',{'bugs':'active'},rawargs=True)
        assert r
    eosapi.produce_block()

