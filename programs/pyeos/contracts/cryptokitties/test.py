import os
import time
import wallet
import eosapi
import initeos

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init():
    with producer:
        if not eosapi.get_account('kitties').permissions:
            r = eosapi.create_account('eosio', 'kitties', initeos.key1, initeos.key2)
            assert r

    with producer:
        r = eosapi.set_contract('kitties','../../programs/pyeos/contracts/cryptokitties/main.py','../../programs/pyeos/contracts/cryptokitties/cryptokitties.abi', 1)
#        r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
        assert r

#eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)

def deploy_all():
    src_dir = '../../programs/pyeos/contracts/cryptokitties'
    files = os.listdir(src_dir)
    for file_name in files:
        if not file_name.endswith('.py'):
            continue
        if file_name in ('main.py', 'test.py'):
            continue
        src_code = open(os.path.join(src_dir, file_name), 'r').read()
        mod_name = file_name
        msg = int.to_bytes(len(mod_name), 1, 'little')
        msg += mod_name.encode('utf8')
        msg += src_code.encode('utf8')
        with producer:
            print('++++++++++++++++deply:', file_name)
            r = eosapi.push_message('kitties','deploy',msg,{'kitties':'active'},rawargs=True)
            assert r

def deploy(src_file):
    src_dir = '../../programs/pyeos/contracts/cryptokitties'

    src_code = open(os.path.join(src_dir, src_file), 'r').read()
    mod_name = src_file
    msg = int.to_bytes(len(mod_name), 1, 'little')
    msg += mod_name.encode('utf8')
    msg += src_code.encode('utf8')
    with producer:
        print('++++++++++++++++deply:', src_file)
        r = eosapi.push_message('kitties','deploy',msg,{'kitties':'active'},rawargs=True)
        assert r


def test(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('kitties','sayhello',name,{'kitties':'active'},rawargs=True)
        assert r

def test2(count):
    import time
    import json
    functions = []
    args = []
    per = {'kitties':'active'}
    for i in range(count):
        functions.append('sayhello')
        arg = str(i)
        args.append(arg)
    ret, cost = eosapi.push_messages('kitties', functions, args, per, True, rawargs=True)
    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()

def call(name=None):
    with producer:
        r = eosapi.push_message('kitties','call','hello,world',{'kitties':'active'},rawargs=True)
        assert r

def import_test():
    import kittybreeding
    import kittycore
    import clockauction
    import clockauctionbase
    import erc720metadata
    import erc721
    import genescienceinterface
    import kitty
    import kittyaccesscontrol
    import kittyauction
    import kittybase
    import kittybreeding
    import kittyminting
    import kittyownership
    import ownable
    import pausable
    import saleclockauction
    import siringclockauction



