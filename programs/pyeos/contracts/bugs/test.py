import time
import wallet
import eosapi
import initeos

from common import init_, producer

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')


def init(func):
    def func_wrapper(*args):
        init_('bugs', 'bugs.py', 'bugs.abi', __file__)
        return func(*args)
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

