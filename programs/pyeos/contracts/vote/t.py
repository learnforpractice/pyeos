import time
import struct

import wallet
import eosapi
import initeos

from common import prepare, producer

import vote

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('vote', 'vote.py', 'vote.abi', __file__)
        return func(*args, **kwargs)
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
        r = eosapi.push_action('vote', 'addproposal', msg, {'vote':'active'})
        assert r

@init
def giveRightToVote(name=None):
    with producer:
        if not name:
            name = 'mike'
        msg = struct.pack('QQ', eosapi.N('vote'), eosapi.N('hello'))
        r = eosapi.push_action('vote', 'giveright', msg, {'vote':'active'})
        assert r

@init
def delegate(name=None):
    with producer:
        if not name:
            name = 'hello'
        msg = struct.pack('QQ', eosapi.N('vote'), eosapi.N(name))
        r = eosapi.push_action('vote', 'delegate', msg, {'vote':'active'})
        assert r
@init
def vote(voter, proposal_index):
    with producer:
        msg = struct.pack('QQ', eosapi.N(voter), proposal_index)
        r = eosapi.push_action('vote', 'vote', msg, {'vote':'active'})
        assert r

@init
def test2(count):
    import time
    import json

    actions = []
    for i in range(count):
        action = ['vote', 'sayhello', {'vote':'active'}, str(i)]
        actions.append(action)

    ret, cost = eosapi.push_actions(actions, True)
    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()


