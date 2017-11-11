import imp
import time
import struct

import eosapi
import eostypes
from eoslib import N
import blockchain
import logging as log

from eostypes import PySignedTransaction, PyMessage

from main.chain_controller import chain_controller


producer = eosapi.Producer()

def test1():

    with producer:
        if not eosapi.get_account('currency'):
                r = eosapi.create_account('inita', 'currency', key1, key2)
                assert r
        r = eosapi.set_contract('currency','../../programs/pyeos/contracts/currency/currency.py','../../contracts/currency/currency.abi',1)
        assert r

    start = time.time()
    ts = PySignedTransaction()
    ts.reqire_scope(b'test')
    ts.reqire_scope(b'currency')

    for i in range(1):
        data = struct.pack("QQQ", N(b'currency'), N(b'test'), 1)
        msg = PyMessage()
        msg.init(b'currency', b'transfer', [[b'currency',b'active']], data)
        ts.add_message(msg)
        
    ctl = chain_controller()
    ctl.process_transaction(ts)
    r = eosapi.get_table(b'test', b'currency', b'account')
    print(r)
    print('cost:',time.time()-start)

def test2():
    with producer:
        if not eosapi.get_account('currency'):
                r = eosapi.create_account('inita', 'currency', key1, key2)
                assert r
        if not eosapi.get_account('test'):
            if not eosapi.get_account('test'):
                r = eosapi.create_account('inita', 'test', key1, key2)
                assert r
        r = eosapi.set_contract('currency','../../contracts/currency/currency.py','../../contracts/currency/currency.abi',1)

    start = time.time()
    ts = PySignedTransaction()
    ts.reqire_scope(b'test')
    ts.reqire_scope(b'currency')

    for i in range(1):
        data = struct.pack("QQQ", N(b'currency'), N(b'test'), 1)
        msg = PyMessage()
        msg.init(b'currency', b'transfer', [[b'currency',b'active']], data)
        ts.add_message(msg)
        
    ctl = chain_controller()
    ctl.process_transaction(ts)
    r = eosapi.get_table(b'test', b'currency', b'account')
    print(r)
    print('cost:',time.time()-start)

