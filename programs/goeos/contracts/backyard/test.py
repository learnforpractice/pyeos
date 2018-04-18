import os
import time
import wallet
import eosapi
import initeos

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init(mpy=True):
    with producer:
        if not eosapi.get_account('backyard').permissions:
            r = eosapi.create_account('eosio', 'backyard', initeos.key1, initeos.key2)
            assert r

    with producer:
        if mpy:
            with open('../../programs/pyeos/contracts/backyard/backyard.mpy', 'wb') as f:
                f.write(eosapi.mp_compile('../../programs/pyeos/contracts/backyard/backyard.py'))
            r = eosapi.set_contract('backyard','../../programs/pyeos/contracts/backyard/backyard.mpy','../../programs/pyeos/contracts/backyard/backyard.abi', 1)
        else:
            r = eosapi.set_contract('backyard','../../programs/pyeos/contracts/backyard/backyard.py','../../programs/pyeos/contracts/backyard/backyard.abi', 1)
#        r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
            assert r

#eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)

def test(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('backyard','sayhello',name,{'backyard':'active'},rawargs=True)
        assert r

def deploy():
    src_dir = os.path.dirname(os.path.abspath(__file__))
    file_name = 'garden.py'
    
    src_code = open(os.path.join(src_dir, file_name), 'rb').read()
    mod_name = file_name
    msg = int.to_bytes(len(mod_name), 1, 'little')
    msg += mod_name.encode('utf8')
    msg += int.to_bytes(0, 1, 'little') # source code
    msg += src_code

    print('++++++++++++++++deply:', file_name)
    r = eosapi.push_message('backyard','deploy',msg,{'backyard':'active'},rawargs=True)
    assert r

    producer.produce_block()

def deploy_mpy():
    src_dir = os.path.dirname(os.path.abspath(__file__))
    file_name = 'garden.py'
    
    src_code = eosapi.mp_compile(os.path.join(src_dir, file_name))
    file_name = file_name.replace('.py', '.mpy')
    mod_name = file_name
    msg = int.to_bytes(len(mod_name), 1, 'little')
    msg += mod_name.encode('utf8')
    msg += int.to_bytes(1, 1, 'little') # compiled code
    msg += src_code

    print('++++++++++++++++deply:', file_name)
    r = eosapi.push_message('backyard','deploy',msg,{'backyard':'active'},rawargs=True)
    assert r

    producer.produce_block()



