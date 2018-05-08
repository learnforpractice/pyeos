import os
import time
import pickle

import os
import sys
import imp
import initeos
import eosapi
import eoslib
from tools import cpp2wast

producer = eosapi.Producer()
CODE_TYPE_WAST = 0
CODE_TYPE_PY = 1
CODE_TYPE_MPY = 2

def prepare(name, src, abi, code_type, full_src_path):
    _src_dir = os.path.dirname(os.path.abspath(full_src_path))
    if code_type == 0:
        cpp2wast.set_src_path(_src_dir)
        cpp_src_file = src.replace('.wast', '.cpp')
        if not cpp2wast.build(cpp_src_file):
            raise Exception("build {0} failed".format(cpp_src_file))

    if src.find('/') < 0:
        src = os.path.join(_src_dir, src)

    if abi and abi.find('/') < 0:
        abi = os.path.join(_src_dir, abi)

    if code_type == CODE_TYPE_MPY:
        mpy_file = src[:-3] + '.mpy'
        with open(mpy_file, 'wb') as f:
            f.write(eosapi.mp_compile(src))
        src = mpy_file

    if not eosapi.get_account(name).permissions:
        r = eosapi.create_account('eosio', name, initeos.key1, initeos.key2)
        assert r

    old_code = eosapi.get_code(name)
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
        elif (code == old_code[1:] or code == old_code):
            need_update = False

    if need_update:
        with producer:
            if code_type == 0:
                r = eosapi.set_contract(name, src, abi, 0)
            else:
                r = eosapi.set_contract(name, src, abi, 1)
            assert r, 'set_contract failed'

class Sync(object):
    def __init__(self, _account, _dir = None, _ignore = []):
        self.ignore_files = _ignore 

        if not isinstance(_account, int):
            self.account = eosapi.N(_account)
        else:
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

        code = self.account
        itr = eoslib.db_find_i64(code, code, code, id)
        if itr < 0:
            return True

        old_src = eoslib.db_get_i64(itr)
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
            r = eosapi.push_message('kitties','deploy',msg,{'kitties':'active'})
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
            r = eosapi.push_message('kitties','deploy',msg,{'kitties':'active'})
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
        r = eosapi.push_message('kitties','deploy',msg,{'kitties':'active'})
        assert r

        producer.produce_block()
        time.sleep(0.2)

    def deploy_all_mpy(self):
        with producer:
            files = os.listdir(self.src_dir)
            for file_name in files:
                if not file_name.endswith('.py'):
                    continue
                if file_name in self.ignore_files:
                    continue
                self.deploy_mpy(file_name)

