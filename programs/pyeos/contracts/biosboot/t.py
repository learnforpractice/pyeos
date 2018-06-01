import os
import json
import numpy

import eosapi
import wallet

from common import producer
import initeos

system_accounts = [
    'eosio.bpay',
    'eosio.msig',
    'eosio.names',
    'eosio.ram',
    'eosio.ramfee',
    'eosio.saving',
    'eosio.stake',
    'eosio.token',
    'eosio.vpay',
]

pub = 'EOS8Znrtgwt8TfpmbVpTKvA2oB8Nqey625CLN8bCN3TEbgx86Dsvr'
prv = '5K463ynhZoCDDa4RDcr63cUwWLTnKqmdcoTKTHBjqoKfv4u5V7p'

accounts_json = os.path.dirname(__file__)
accounts_json = os.path.join(accounts_json, '../../../..', 'tutorials/bios-boot-tutorial/accounts.json')

with open(accounts_json) as f:
    a = json.load(f)
    users = a['users']
    producers = a['producers']
    firstProducer = len(users)
    numProducers = len(producers)
    accounts = a['users'][:1000] + a['producers']

print(accounts[0])
print(len(accounts))

def create_accounts():
    step = 500
    for i in range(0, len(accounts), step):
        with producer:
            for j in range(step):
                if i+j >= len(accounts):
                    break
                a = accounts[i+j]
                print(a, i, j)
                if not eosapi.get_account(a['name']):
                    eosapi.create_account('eosio', a['name'], a['pub'], a['pub'])

def create_sys_account():
    with producer:
        for a in system_accounts:
            if not eosapi.get_account(a):
                eosapi.create_account('eosio', a, pub, pub)

def import_keys():
    keys = wallet.list_keys('mywallet', initeos.psw)
    for a in accounts:
        if not a['pub'] in keys:
            wallet.import_key('mywallet', a['pvt'], False)
    wallet.save('mywallet')

def allocate_funds():
    b = 0
    e = len(accounts)
    dist = numpy.random.pareto(1.161, e - b).tolist() # 1.161 = 80/20 rule
    dist.sort()
    dist.reverse()
    factor = 1_000_000_000 / sum(dist)
    total = 0
    for i in range(b, e):
        funds = round(factor * dist[i - b] * 10000)
        if i >= firstProducer and i < firstProducer + numProducers:
            funds = max(funds, round(args.min_producer_funds * 10000))
        total += funds
        accounts[i]['funds'] = funds
    return total

def int_to_currency(i):
    return '%d.%04d %s' % (i // 10000, i % 10000, 'EOS')


def create_tokens():

    msg = {"issuer":"eosio","maximum_supply":"10000000000.0000 EOS"}
    r, cost = eosapi.push_action('eosio.token', 'create', msg, {'eosio.token':'active'})
    assert r

    total_allocation = allocate_funds()

    r, cost = eosapi.push_action('eosio.token','issue',{"to":"eosio","quantity":int_to_currency(total_allocation),"memo":""},{'eosio':'active'})
    assert r


def all():
    create_accounts()
    create_sys_account()
    import_keys()
    allocate_funds()
    create_tokens()
    
