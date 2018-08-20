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

def assert_ret(rr):
    for r in rr:
        if r['except']:
            print(r['except'])
        assert not r['except']

def u():
    print('upgrading...')
    upgrade_file = os.path.join(os.path.dirname(__file__), 'upgrade.py')
    with open(upgrade_file) as f:
        script = f.read()
    args = {'version':111, 'script':script}
    args = eosapi.pack_args('eosio', 'upgrade', args)
    act = ['eosio', 'upgrade', {'eosio':'active'}, args]
    rr, cost = eosapi.push_actions([act])
    assert_ret(rr)

#eosapi.push_action('aaaa11t', 'boost', {'account':'aaaa11t'}, {'aaaa11t':'active'})


