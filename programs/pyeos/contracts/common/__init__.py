import os
import time
import pickle

import os
import db
import sys
import imp
import initeos
import eosapi
from tools import cpp2wast

producer = eosapi.Producer()
CODE_TYPE_WAST = 0
CODE_TYPE_PY = 1
CODE_TYPE_EVM = 2

def assert_ret(r):
    if r['except']:
        print(r['except'])
    assert not r['except']

def _create_account(account):
    actions = []
    newaccount = {'creator': 'eosio',
     'name': account,
     'owner': {'threshold': 1,
               'keys': [{'key': initeos.key1,
                         'weight': 1}],
               'accounts': [],
               'waits': []},
     'active': {'threshold': 1,
                'keys': [{'key': initeos.key2,
                          'weight': 1}],
                'accounts': [],
                'waits': []}}

    _newaccount = eosapi.pack_args('eosio', 'newaccount', newaccount)
    act = ['eosio', 'newaccount', _newaccount, {'eosio':'active'}]
    actions.append(act)
    '''
    args = {'payer':'eosio', 'receiver':account, 'quant':"1.0000 EOS"}
    args = eosapi.pack_args('eosio', 'buyram', args)
    act = ['eosio', 'buyram', {'eosio':'active'}, args]
    actions.append(act)
    '''
    if eosapi.get_code('eosio')[0]:
        args = {'payer':'eosio', 'receiver':account, 'bytes':64*1024}
        args = eosapi.pack_args('eosio', 'buyrambytes', args)
        act = ['eosio', 'buyrambytes', args, {'eosio':'active'}]
        actions.append(act)
    
        args = {'from': 'eosio',
         'receiver': account,
         'stake_net_quantity': '1.0050 EOS',
         'stake_cpu_quantity': '1.0050 EOS',
         'transfer': 1}
        args = eosapi.pack_args('eosio', 'delegatebw', args)
        act = ['eosio', 'delegatebw', args, {'eosio':'active'}]
        actions.append(act)

    rr, cost = eosapi.push_actions(actions)
    assert_ret(rr)

def _set_contract(account, wast_file, abi_file):
    with open(wast_file, 'rb') as f:
        wasm = eosapi.wast2wasm(f.read())

    code_hash = eosapi.sha256(wasm)
    with open(abi_file, 'rb') as f:
        abi = f.read()

    actions = []
    _setcode = eosapi.pack_args('eosio', 'setcode', {'account':account,'vmtype':0, 'vmversion':0, 'code':wasm.hex()})
#        _setabi = eosapi.pack_args('eosio', 'setabi', {'account':account, 'abi':abi.hex()})
    _setabi = eosapi.pack_setabi(abi_file, account)

    old_hash = eosapi.get_code_hash(account)
    print(old_hash, code_hash)
    if code_hash != old_hash:
        setcode = ['eosio', 'setcode', _setcode, {account:'active'}]
        actions.append(setcode)

    setabi = ['eosio', 'setabi', _setabi, {account:'active'}]
    actions.append(setabi)
    rr, cost = eosapi.push_actions(actions)
    assert_ret(rr)

def build_native(account, full_src_path):
    _src_dir = os.path.dirname(os.path.abspath(full_src_path))
    cpp2wast.set_src_path(_src_dir)
    cpp2wast.build_native(account+'.cpp', account)
    lib_file = os.path.join(_src_dir, 'lib{0}.dylib'.format(account))
    debug.set_debug_contract(account, lib_file)

def prepare(account, src, abi, full_src_path, code_type = None):
    _src_dir = os.path.dirname(os.path.abspath(full_src_path))
    if not code_type:
        if src.endswith('.wast'):
            code_type = CODE_TYPE_WAST
        elif src.endswith('.py'):
            code_type = CODE_TYPE_PY
        else:
            raise Exception('unknown code type')

    if code_type == 0:
        cpp2wast.set_src_path(_src_dir)
        cpp_src_file = src.replace('.wast', '.cpp')
        if not cpp2wast.build(cpp_src_file):
            raise Exception("build {0} failed".format(cpp_src_file))

    if src.find('/') < 0:
        src = os.path.join(_src_dir, src)

    if abi and abi.find('/') < 0:
        abi = os.path.join(_src_dir, abi)


    if not eosapi.get_account(account):
        print('*'*20, 'create_account')
        _create_account(account)

    old_code = eosapi.get_code(account)
    need_update = True
    if old_code:
        old_code = old_code[0]
        with open(src, 'rb') as f:
            code = f.read()
        if code_type == CODE_TYPE_WAST:
            code = eosapi.wast2wasm(code)
            old_code = eosapi.wast2wasm(old_code)
            if code == old_code:
                need_update = False
        elif CODE_TYPE_PY == code_type:
            code = eosapi.mp_compile(src)
            if code == old_code[1:]:
                need_update = False
        elif (code == old_code[1:] or code == old_code):
            need_update = False

    if need_update:
        print('Updating contract', src)
        if code_type == 0:
            _set_contract(account, src, abi)
        else:
            r = eosapi.set_contract(account, src, abi, code_type)
            assert r, 'set_contract failed'

class Sync(object):
    def __init__(self, _account, _dir = None, _ignore = []):
        self.ignore_files = _ignore 

        self.account = _account

        if _dir:
            if not os.path.exists(_dir):
                raise Exception(_dir + ' not found')
            self.src_dir = _dir
        else:
            self.src_dir = os.path.dirname(os.path.abspath(__file__))

    def need_update(self, src_dir, src_file, mpy=False):
        if mpy:
            id = eosapi.hash64(src_file.replace('.py', '.mpy'))
        else:
            id = eosapi.hash64(src_file)

        code = eosapi.N(self.account)
        itr = db.find_i64(code, code, code, id)
        if itr < 0:
            return True

        old_src = db.get_i64(itr)
        _full_file_path = os.path.join(src_dir, src_file)

        if not mpy:
            with open(_full_file_path, 'rb') as f:
                if old_src[1:] == f.read():
                    return False
        else:
            src_code = eosapi.mp_compile(_full_file_path)
            if old_src[1:] == src_code:
                return False
        return True

    def deploy(self, src_file, _dir = None):
        _abs_file_path = None
        if not _dir:
            _dir = self.src_dir

        _abs_file_path = os.path.join(_dir, src_file)

        if not self.need_update(_dir, src_file):
            print('++++++++++:', src_file, "up to date")
            return

        src_code = open(_abs_file_path, 'rb').read()
        mod_name = src_file #os.path.basename(src_file)
        msg = int.to_bytes(len(mod_name), 1, 'little')
        msg += mod_name.encode('utf8')
        msg += int.to_bytes(0, 1, 'little') # source code
        msg += src_code
        with producer:
            print('++++++++++++++++deply:', mod_name)
            r = eosapi.push_action(self.account, 'deploy',msg,{self.account:'active'})
            assert r

    def deploy_depend_libs(self, _libs_dir, depend_libs):
        for file_name in depend_libs:
            _src_code_path = os.path.join(_libs_dir, file_name)
            if not self.need_update(_libs_dir, file_name):
                continue
            src_code = open(_src_code_path, 'r').read()
            mod_name = file_name
            msg = int.to_bytes(len(mod_name), 1, 'little')
            msg += mod_name.encode('utf8')
            msg += int.to_bytes(0, 1, 'little') # source code
            msg += src_code.encode('utf8')

            print('++++++++++++++++deply:', file_name)
            r = eosapi.push_action(self.account,'deploy',msg,{self.account:'active'})
            assert r
        producer.produce_block()

    def deploy_all(self):
        files = os.listdir(self.src_dir)
        for file_name in files:
            _abs_file_path = os.path.join(self.src_dir, file_name)
            if not file_name.endswith('.py'):
                continue
            if file_name in self.ignore_files:
                continue
            self.deploy(file_name)
#('main.py', 'test.py', 'ustruct.py', 'eoslib.py')

    def deploy_mpy(self, file_name):
        _abs_file_path = os.path.join(self.src_dir, file_name)
        if not self.need_update(self.src_dir, file_name, mpy=True):
            print('++++++++++:', file_name, "up to date")
            return

        src_code = eosapi.mp_compile(_abs_file_path)
        file_name = file_name.replace('.py', '.mpy')
        mod_name = file_name
        msg = int.to_bytes(len(mod_name), 1, 'little')
        msg += mod_name.encode('utf8')
        msg += int.to_bytes(1, 1, 'little') # compiled code
        msg += src_code

        print('++++++++++++++++deply:', file_name)
        r = eosapi.push_action(self.account,'deploy',msg,{self.account:'active'})
        assert r

    def deploy_all_mpy(self):
        with producer:
            files = os.listdir(self.src_dir)
            for file_name in files:
                if not file_name.endswith('.py'):
                    continue
                if file_name in self.ignore_files:
                    continue
                self.deploy_mpy(file_name)

    def deploy_native(self, contract, version, file_name):
        contract = eosapi.N(contract)
        msg = int.to_bytes(contract, 8, 'little')
        msg += int.to_bytes(version, 4, 'little')
        with open(file_name, 'rb') as f:
            src_code = f.read()
        msg += src_code

        print('++++++++++++++++deply:', file_name)
        r = eosapi.push_action(self.account,'deploy',msg,{self.account:'active'})
        assert r

    def deploy_vm(self, vm_name, type, version, file_name):
        vm_name = eosapi.N(vm_name)
        account = eosapi.N(self.account)
        msg = int.to_bytes(account, 8, 'little') #scope
        msg += int.to_bytes(account, 8, 'little') #table
        msg += int.to_bytes(vm_name, 8, 'little') #id
        msg += int.to_bytes(type, 4, 'little')
        msg += int.to_bytes(version, 4, 'little')
        with open(file_name, 'rb') as f:
            src_code = f.read()
        msg += src_code

        print('++++++++++++++++deply:', file_name)
        r = eosapi.push_action(self.account,'deploy',msg,{self.account:'active'})
        assert r

