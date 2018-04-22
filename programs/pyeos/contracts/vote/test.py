import time
import wallet
import eosapi
import initeos

from common import init_, producer

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init(func):
    def func_wrapper(*args):
        init_('vote', 'vote.py', 'vote.abi', __file__)
        return func(*args)
    return func_wrapper


@init
def addProposal(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('vote','addproposal',name,{'vote':'active'},rawargs=True)
        assert r

@init
def giveRightToVote(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('vote','giveright',name,{'vote':'active'},rawargs=True)
        assert r

@init
def delegate(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('vote','delegate',name,{'vote':'active'},rawargs=True)
        assert r
@init
def vote(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('vote','vote',name,{'vote':'active'},rawargs=True)
        assert r

@init
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
        contracts.append('vote')
        per.append({'vote':'active'})
    ret = eosapi.push_messages(contracts, functions, args, per, True, rawargs=True)
    assert ret
    cost = ret['cost_time']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()


