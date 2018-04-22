import time
import struct

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
def test_all():
    addProposal('mike')
    addProposal('jack')
    giveRightToVote('hello')
    delegate('john')
    vote('hello', 0)

@init
def addProposal(name=None):
    with producer:
        if not name:
            name = 'mike'
        sender = eosapi.N('hello')
        msg = int.to_bytes(sender, 8, 'little')
        msg += 'mike'.encode('utf8')
        r = eosapi.push_message('vote', 'addproposal', msg, {'vote':'active'}, rawargs=True)
        assert r

@init
def giveRightToVote(name=None):
    with producer:
        if not name:
            name = 'mike'
        msg = struct.pack('QQ', eosapi.N('vote'), eosapi.N('hello'))
        r = eosapi.push_message('vote', 'giveright', msg, {'vote':'active'}, rawargs=True)
        assert r

@init
def delegate(name=None):
    with producer:
        if not name:
            name = 'hello'
        msg = struct.pack('QQ', eosapi.N('vote'), eosapi.N(name))
        r = eosapi.push_message('vote', 'delegate', msg, {'vote':'active'}, rawargs=True)
        assert r
@init
def vote(voter, proposal_index):
    with producer:
        msg = struct.pack('QQ', eosapi.N(voter), proposal_index)
        r = eosapi.push_message('vote', 'vote', msg, {'vote':'active'}, rawargs=True)
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


