import os
import time
import wallet
import eosapi
import initeos
import db
from common import prepare, producer
from tools import cpp2wast

def init(func):
    def func_wrapper(*args, **kwargs):
        cpp2wast.set_src_path(os.path.dirname(__file__))
        if not cpp2wast.build('math.cpp'):
            raise Exception("build {0} failed".format('math.cpp'))

        prepare('backyard', 'backyard.py', 'backyard.abi', __file__)
        return func(*args, **kwargs)
    return func_wrapper

@init
def test(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_action('backyard','sayhello',name,{'backyard':'active'})
        assert r

libs = ('asset.py', 'token.py', 'cache.py', 'garden.py', 'solidity.py') #'storage.py',

@init
def deploy():
    src_dir = os.path.dirname(os.path.abspath(__file__))

    code = eosapi.N('backyard')
    for file_name in libs:
        src_code = open(os.path.join(src_dir, file_name), 'rb').read()
        src_id = eosapi.hash64(file_name, 0)
        itr = db.find_i64(code, code, code, src_id)
        if itr >= 0:
            old_src = db.get_i64(itr)
            if old_src[1:] == src_code:
                continue
        mod_name = file_name
        msg = int.to_bytes(len(mod_name), 1, 'little')
        msg += mod_name.encode('utf8')
        msg += int.to_bytes(0, 1, 'little') # source code
        msg += src_code
    
        print('++++++++++++++++deply:', file_name)
        r = eosapi.push_action('backyard','deploy',msg,{'backyard':'active'})
        assert r

    producer.produce_block()

@init
def deploy_mpy():
    src_dir = os.path.dirname(os.path.abspath(__file__))
    code = eosapi.N('backyard')
    for file_name in libs:
        print('deploy ', file_name)
        src_code = eosapi.mp_compile(os.path.join(src_dir, file_name))
        file_name = file_name.replace('.py', '.mpy')
        src_id = eosapi.hash64(file_name, 0)
        itr = db.find_i64(code, code, code, src_id)
        if itr >= 0:
            #Something went wrong in mp_compile.
            #Generating different bytecode each time with the same source code, walkaround this problem for now
            continue 
            old_src = db.get_i64(itr)
            if old_src[1:] == src_code:
                continue
        mod_name = file_name
        msg = int.to_bytes(len(mod_name), 1, 'little')
        msg += mod_name.encode('utf8')
        msg += int.to_bytes(1, 1, 'little') # compiled python bytecode
        msg += src_code

        print('++++++++++++++++deply:', file_name)
        r = eosapi.push_action('backyard','deploy',msg,{'backyard':'active'})
        assert r

@init
def deploy_wast():
    src_dir = os.path.dirname(os.path.abspath(__file__))
    code = eosapi.N('backyard')
    for file_name in ('math.wast',):
        print('deploy ', file_name)
        bin_code = None
        with open(os.path.join(src_dir, file_name), 'rb') as f:
            bin_code = eosapi.wast2wasm(f.read())

        src_id = eosapi.hash64(file_name, 0)
        itr = db.find_i64(code, code, code, src_id)
        if itr >= 0:
            old_src = db.get_i64(itr)
            if old_src[1:] == bin_code:
                continue
        mod_name = file_name
        msg = int.to_bytes(len(mod_name), 1, 'little')
        msg += mod_name.encode('utf8')
        msg += int.to_bytes(3, 1, 'little')# compiled cpp bytecode
        msg += bin_code
        print(msg)
        print('++++++++++++++++deply:', file_name)
        r = eosapi.push_action('backyard','deploy',msg,{'backyard':'active'})
        assert r


@init
def load_all():
    for lib in libs:
        r = eosapi.push_action('backyard','load', lib[:-3] ,{'backyard':'active'})

@init
def call_wasm():
    deploy_wast()
    eosapi.push_action('backyard', 'callwasm', b'' ,{'backyard':'active'})
    
