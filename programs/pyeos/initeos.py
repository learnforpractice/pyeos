import os
import sys
import wallet
import eosapi

key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'

def init():

    psw = 'PW5Kk1h8RqDwf8CB8mKcz7WZ8r6MF9of9CYvrC96XdBL5Z1SwBVx9'
    
    if not os.path.exists('config-dir/mywallet.wallet'):
        psw = wallet.create('mywallet')
        print(psw)
    
    wallet.open('mywallet')
    wallet.unlock('mywallet',psw)
    
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

    src_dir = os.path.dirname(os.path.abspath(__file__))
    sys.path.append(os.path.join(src_dir, '/contracts/cryptokitties'))
    contracts_path = os.path.join(src_dir, '../../build', 'contracts') 
    for account in ['eosio.bios', 'eosio.msig', 'eosio.system', 'eosio.token']:
        if not eosapi.get_account(account).permissions:
            r = eosapi.create_account('eosio', account, key1, key2)
            assert r
        if not eosapi.get_code(account):
            wast = os.path.join(contracts_path, account, account+'.wast')
            abi = os.path.join(contracts_path, account, account+'.abi')
            r = eosapi.set_contract(account, wast, abi,0)
        eosapi.produce_block()
