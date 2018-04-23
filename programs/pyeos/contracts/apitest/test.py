import time
import wallet
import eosapi
import initeos

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

from common import init_, producer

def init(func):
    def func_wrapper(*args):
        init_('apitest', 'apitest.py', 'apitest.abi', __file__)
        return func(*args)
    return func_wrapper

@init
def test():
    with producer:
        r = eosapi.push_message('apitest','dbtest','',{'apitest':'active'},rawargs=True)
        assert r

@init
def inline_send():
    with producer:
        r = eosapi.push_message('apitest', 'inlinesend', '', {'apitest':'active'}, rawargs=True)
        assert r
