import os
import re
import sys
import imp
import time
import struct

import debug
import wallet
import eosapi
import initeos
import traceback
from eosapi import N, mp_compile, pack_bytes, pack_setabi, push_transactions
from common import prepare, producer

def gen_names(n):
    prefix = 'aaaa'
    chs = '12345abcdefghijklmnopqrstuvwxyz'
    names = []
    for i in range(n):
        name = prefix + chs[int(i/31/31)%31] + chs[int((i/31)%31)] + chs[i%31]
        names.append(name)
    return names

def assert_ret(rr):
    for r in rr:
        if r['except']:
            print(r['except'])
        assert not r['except']

def t():
    contracts_path = os.path.join(os.getcwd(), '..', 'contracts')
    sys.path.append(os.getcwd())
    account = 'eosio'
    path = 'eosio.system'
    _path = os.path.join(contracts_path, path, path)
    print('+++++++++code update', account)
    wast = _path + '.wast'
    abi = _path + '.abi'
    with producer:
        r = eosapi.set_contract(account, wast, abi, 0)

def t1():
    systemAccounts = [
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
    newaccount = {'creator':'eosio', 
                  'name': '',
                  'owner': {
                            "threshold": 1,
                            "keys": [
                                {
                                    "key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
                                    "weight": 1
                                }
                            ],
                            "accounts": [],
                            "waits": []
                            },
                  'active':{
                        "threshold": 1,
                        "keys": [
                            {
                                "key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
                                "weight": 1
                            }
                        ],
                        "accounts": [],
                        "waits": []
                    },
                }
    for account in systemAccounts:
        if not eosapi.get_account(account):
            actions = []
            print('+++++++++create account', account)
            newaccount['name'] = account
            _newaccount = eosapi.pack_args('eosio', 'newaccount', newaccount)
            act = ['eosio', 'newaccount', {'eosio':'active'}, _newaccount]
            actions.append(act)
            rr, cost = eosapi.push_actions(actions)
            for r in rr:
                if  r['except']:
                    print(r['except'])
                assert r and not r['except']


def t2():
    contracts_path = os.path.join(os.getcwd(), '..', 'contracts')
    sys.path.append(os.getcwd())
    account = 'eosio'
    path = 'eosio.system'
    accounts = gen_names(100)


    _path = os.path.join(contracts_path, path, path)
    wast = _path + '.wast'
    abi_file = _path + '.abi'

    with open(wast, 'rb') as f:
        wasm = eosapi.wast2wasm(f.read())
    code_hash = eosapi.sha256(wasm)
    with open(abi_file, 'rb') as f:
        abi = f.read()

    newaccount = {'creator':'eosio', 
                  'name': '',
                  'owner': {
                            "threshold": 1,
                            "keys": [
                                {
                                    "key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
                                    "weight": 1
                                }
                            ],
                            "accounts": [],
                            "waits": []
                            },
                  'active':{
                        "threshold": 1,
                        "keys": [
                            {
                                "key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
                                "weight": 1
                            }
                        ],
                        "accounts": [],
                        "waits": []
                    },
                }

    for account in accounts:
        if not eosapi.get_account(account):
            actions = []
            print('+++++++++create account', account)
            '''
            args = {'payer':'eosio', 'receiver':account, 'quant':"1.0000 EOS"}
            args = eosapi.pack_args('eosio', 'buyram', args)
            act = ['eosio', 'buyram', {'eosio':'active'}, args]
            actions.append(act)
            '''

            newaccount['name'] = account
            _newaccount = eosapi.pack_args('eosio', 'newaccount', newaccount)
            act = ['eosio', 'newaccount', {'eosio':'active'}, _newaccount]
            actions.append(act)
            rr, cost = eosapi.push_actions(actions)
            for r in rr:
                if  r['except']:
                    print(r['except'])
                assert r and not r['except']

    for account in accounts:
        print('+++++++++code update', account)
        actions = []
        _setcode = eosapi.pack_args('eosio', 'setcode', {'account':account,'vmtype':0, 'vmversion':0, 'code':wasm.hex()})
#        _setabi = eosapi.pack_args('eosio', 'setabi', {'account':account, 'abi':abi.hex()})
        _setabi = pack_setabi(abi_file, account)

        old_hash = eosapi.get_code_hash(account)
        print(old_hash, code_hash)
        if code_hash != old_hash:
            setcode = ['eosio', 'setcode', {account:'active'}, _setcode]
            actions.append(setcode)

        setabi = ['eosio', 'setabi', {account:'active'}, _setabi]
        actions.append(setabi)
        rr, cost = eosapi.push_actions(actions)
        for r in rr:
            if  r['except']:
                print(r['except'])
            assert r and not r['except']

def t3():
    accounts = gen_names(100)
    actions = []
    for account in accounts:
        print('++++++boost account', account)
        msg = eosapi.pack_args('eosio', 'boost', {'account':account})
        act = ['eosio', 'boost', {'eosio':'active'}, msg]
        actions.append(act)
    rr, cost = eosapi.push_actions(actions)
    assert_ret(rr)

def t4():
    accounts = gen_names(100)
    actions = []
    for account in accounts[10:80]:
        print('++++++boost account', account)
        msg = eosapi.pack_args('eosio', 'cancelboost', {'account':account})
        act = ['eosio', 'cancelboost', {'eosio':'active'}, msg]
        actions.append(act)
    rr, cost = eosapi.push_actions(actions)
    assert_ret(rr)


def buyram():
    accounts = gen_names(100)
    actions = []
    for account in accounts:
        print('buy ram', account)
        args = {'payer':'eosio', 'receiver':account, 'quant':"1.0000 EOS"}
        args = eosapi.pack_args('eosio', 'buyram', args)
        act = ['eosio', 'buyram', {'eosio':'active'}, args]
        actions.append(act)
    rr, cost = eosapi.push_actions(actions)
    assert_ret(rr)

def buyrambytes():
    accounts = gen_names(100)
    actions = []
    for account in accounts:
        print('buy ram in bytes', account)
        args = {'payer':'eosio', 'receiver':account, 'bytes':1000000}
        args = eosapi.pack_args('eosio', 'buyrambytes', args)
        act = ['eosio', 'buyrambytes', {'eosio':'active'}, args]
        actions.append(act)
    rr, cost = eosapi.push_actions(actions)
    assert_ret(rr)

#eosapi.push_action('aaaa11t', 'boost', {'account':'aaaa11t'}, {'aaaa11t':'active'})


