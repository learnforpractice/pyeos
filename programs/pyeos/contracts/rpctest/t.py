import time
import wallet
import eosapi
import initeos

from common import prepare, producer

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('rpctest', 'rpctest.py', 'rpctest.abi', __file__)
        return func(*args, **kwargs)
    return func_wrapper

@init
def test(name=None):
    with producer:
        print('rpctest')
        r = eosapi.push_action('rpctest','sayhello', 'hello,wwww',{'rpctest':'active'})
        assert r

@init
def test2(count):
    import time
    import json
    
    actions = []
    for i in range(count):
        action = ['rpctest', 'sayhello', {'rpctest':'active'}, str(i)]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions, True)
    assert ret[0]
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()


