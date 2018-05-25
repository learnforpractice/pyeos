import os
import re
import sys
import imp
import traceback

import wallet
import eosapi
import debug

from code import InteractiveConsole
from tools import sketch
from imp import reload

producer = eosapi.Producer()

sys.path.insert(0, '/Applications/Eclipse.app/Contents//Eclipse/plugins/org.python.pydev_5.9.2.201708151115/pysrc')

config = '''
# Track only transactions whose scopes involve the listed accounts. Default is to track all transactions.
# filter_on_accounts = 

# Limits the maximum time (in milliseconds) processing a single get_transactions call.
#get-transactions-time-limit = 3

# File to read Genesis State from
genesis-json = genesis.json


# Minimum size MB of database shared memory file
shared-file-size = 1024

 # Enable production on a stale chain, since a single-node test chain is pretty much always stale
enable-stale-production = true
# Enable block production with the testnet producers
producer-name = eosio

private-key = ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3"]


# Load the block producer plugin, so you can produce blocks
plugin = eosio::producer_plugin
# Wallet plugin
plugin = eosio::wallet_api_plugin
# As well as API and HTTP plugins
plugin = eosio::chain_api_plugin
plugin = eosio::http_plugin
plugin = eosio::history_plugin
plugin = eosio::history_api_plugin
'''

genesis = '''
{
  "initial_timestamp": "2018-03-01T12:00:00.000",
  "initial_key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
  "initial_configuration": {
    "base_per_transaction_net_usage": 100,
    "base_per_transaction_cpu_usage": 500,
    "base_per_action_cpu_usage": 1000,
    "base_setcode_cpu_usage": 2097152,
    "per_signature_cpu_usage": 100000,
    "per_lock_net_usage": 32,
    "context_free_discount_cpu_usage_num": 20,
    "context_free_discount_cpu_usage_den": 100,
    "max_transaction_cpu_usage": 10485760,
    "max_transaction_net_usage": 104857,
    "max_block_cpu_usage": 104857600,
    "target_block_cpu_usage_pct": 1000,
    "max_block_net_usage": 1048576,
    "target_block_net_usage_pct": 1000,
    "max_transaction_lifetime": 3600,
    "max_transaction_exec_time": 0,
    "max_authority_depth": 6,
    "max_inline_depth": 4,
    "max_inline_action_size": 4096,
    "max_generated_transaction_count": 16
    "max-transaction-time": 1000
  },
  "initial_chain_id": "0000000000000000000000000000000000000000000000000000000000000000"
}
'''


class PyEosConsole(InteractiveConsole):
    def __init__(self, locals):
        super(PyEosConsole, self).__init__(locals=locals, filename="<console>")

    def check_module(self):
        for module in sys.modules.values():
            if not hasattr(module, '__file__'):
                continue
            if module.__file__.endswith('.py'):
                dir_name = os.path.dirname(module.__file__)
                cache_file = os.path.join(dir_name, '__pycache__', os.path.basename(module.__file__)[:-3]+'.cpython-36.pyc')
                t1 = os.path.getmtime(module.__file__)
                t2 = os.path.getmtime(cache_file)
                if t1 > t2:
                    print('Reloading ', module.__file__)
                    imp.reload(module)

    def interact(self, banner=None, exitmsg=None):
        try:
            sys.ps1
        except AttributeError:
            sys.ps1 = ">>> "
        try:
            sys.ps2
        except AttributeError:
            sys.ps2 = "... "
        cprt = 'Type "help", "copyright", "credits" or "license" for more information.'
        if banner is None:
            self.write("Python %s on %s\n%s\n(%s)\n" %
                       (sys.version, sys.platform, cprt,
                        self.__class__.__name__))
        elif banner:
            self.write("%s\n" % str(banner))
        more = 0
        while 1:
            try:
                if more:
                    prompt = sys.ps2
                else:
                    prompt = sys.ps1
                try:
                    line = self.raw_input(prompt)
                except EOFError:
                    self.write("\n")
                    break
                else:
                    if line.strip():
                        self.check_module()
                    more = self.push(line)
            except KeyboardInterrupt:
                self.write("\nKeyboardInterrupt\n")
                self.resetbuffer()
                more = 0
        if exitmsg is None:
            self.write('now exiting %s...\n' % self.__class__.__name__)
        elif exitmsg != '':
            self.write('%s\n' % exitmsg)

if not hasattr(sys, 'argv'):
    sys.argv = ['']

key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'

def preinit():
    config_file = 'config-dir/config.ini'
    genesis_file = 'config-dir/genesis.json'

    if not os.path.exists(config_file):
        with open(config_file, 'w') as f:
            f.write(config)

    if not os.path.exists(genesis_file):
        with open(genesis_file, 'w') as f:
            f.write(genesis)

from apitest import t as at
from cryptokitties import t as kt
from currency import t as ct
from counter import t as ct1
from credit import t as ct2
from hello import t as ht
from backyard import t as bt
from rpctest import t as rt
from vote import t as vt
from simpleauction import t as st
from lab import t as lt

def init_wallet():
    if not os.path.exists('data-dir/mywallet.wallet'):
        psw = wallet.create('mywallet')
        print('wallet password:', psw)

    wallet.open('mywallet')
    psw = 'PW5K87AKbRvFFMJJm4dU7Zco4fi6pQtygEU4iyajwyTvmELUDnFBK'
    wallet.unlock('mywallet',psw)

def init():
    init_wallet()

    priv_keys = [   
                    '5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3',
                    '5JEcwbckBCdmji5j8ZoMHLEUS8TqQiqBG1DRx1X9DN124GUok9s',
                    '5JbDP55GXN7MLcNYKCnJtfKi9aD2HvHAdY7g8m67zFTAFkY1uBB'
                ]
    
    keys = wallet.list_keys()
    exist_priv_keys = keys.values()
    for priv_key in priv_keys:
        if not priv_key in exist_priv_keys:
            wallet.import_key('mywallet', priv_key)

    if eosapi.is_replay():
        return

    src_dir = os.path.dirname(os.path.abspath(__file__))

    '''
    r = eosapi.push_action('eosio.token', 'create', {"to":"eosio", "quantity":"10000.0000 EOS", "memo":""},{'eosio':'active'})
    r = eosapi.push_action('eosio.token','issue',{"to":"hello","quantity":"1000.0000 EOS","memo":""},{'hello':'active'})
    assert r
    msg = {"from":"eosio", "to":"hello", "quantity":"25.0000 EOS", "memo":"m"}
    r = eosapi.push_action('eosio.token', 'transfer', msg, {'eosio':'active'})
    assert r
    '''

    contracts_path = os.path.join(os.getcwd(), '..', 'contracts')
    sys.path.append(os.getcwd())
    for account in ['eosio.bios', 'eosio.msig', 'eosio.system', 'eosio.token']:
        print('account', account)
        if not eosapi.get_account(account):
            with producer:
                r = eosapi.create_account('eosio', account, key1, key2)
                assert r

        old_code = eosapi.get_code(account)
        if old_code:
            old_code = old_code[0]
        need_update = not old_code
        if old_code:
            print('+++++++++old_code[:4]', old_code[:4])
            if old_code[:4] != b'\x00asm':
                old_code = eosapi.wast2wasm(old_code)

            wast = os.path.join(contracts_path, account, account+'.wast')
            code = open(wast, 'rb').read()
            code = eosapi.wast2wasm(code)

            print(len(code), len(old_code), old_code[:20])
            if code == old_code:
                need_update = False
        if need_update:
            print('+++++++++code update', account)
            wast = os.path.join(contracts_path, account, account+'.wast')
            abi = os.path.join(contracts_path, account, account+'.abi')
            with producer:
                r = eosapi.set_contract(account, wast, abi, 0)

            if account == 'eosio.token':
#                msg = {"issuer":"eosio","maximum_supply":"1000000000.0000 EOS","can_freeze":0,"can_recall":0, "can_whitelist":0}
                with producer:
                    msg = {"issuer":"eosio","maximum_supply":"1000000000.0000 EOS"}
                    r = eosapi.push_action('eosio.token', 'create', msg, {'eosio.token':'active'})
                    assert r
                    r = eosapi.push_action('eosio.token','issue',{"to":"eosio","quantity":"1000.0000 EOS","memo":""},{'eosio':'active'})
                    assert r
                    msg = {"from":"eosio", "to":"hello", "quantity":"100.0000 EOS", "memo":"m"}
                    r = eosapi.push_action('eosio.token', 'transfer', msg, {'eosio':'active'})

    from backyard import t
    t.deploy_mpy()
    #load common libraries
#    t.load_all()

    console = PyEosConsole(locals = globals())
    console.interact(banner='Welcome to PyEos')
