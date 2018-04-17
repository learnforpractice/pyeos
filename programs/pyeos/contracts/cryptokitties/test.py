import os
import time
import wallet
import eosapi
import initeos
import subprocess
import pickle

from common import init_, producer

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init(func):
    def func_wrapper(*args):
        init_('kitties', 'main.py', 'cryptokitties.abi', __file__, mpy=True)
        return func(*args)
    return func_wrapper

class Sync(object):
    def __init__(self, _dir = None, _ignore = []):
        self.last_sync = {}
        self.ignore_files = _ignore 
        if _dir:
            if not os.path.exists(_dir):
                raise Exception(_dir + ' not found')
            self.src_dir = _dir
        else:
            self.src_dir = os.path.dirname(os.path.abspath(__file__))

        self._sync_file = os.path.join(self.src_dir, '.last_sync')

        try:
            with open(self._sync_file, 'rb') as f:
                self.last_sync = pickle.load(f)
        except:
            pass

    def check(self, src_code):
        if not src_code in self.last_sync:
            self.last_sync[src_code] = os.path.getmtime(src_code)
            return False
        if self.last_sync[src_code] == os.path.getmtime(src_code):
            return True
        else:
            self.last_sync[src_code] = os.path.getmtime(src_code)
            return False

    def save(self):
        with open(self._sync_file, 'wb') as f:
            pickle.dump(self.last_sync, f)

    def deploy(self, src_file, _dir = None):
        _abs_file_path = None
        if _dir:
            _abs_file_path = os.path.join(_dir, src_file)
        else:
            _abs_file_path = os.path.join(self.src_dir, src_file)

        if self.check(_abs_file_path):
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
            r = eosapi.push_message('kitties','deploy',msg,{'kitties':'active'},rawargs=True)
            assert r
        self.save()

    def deploy_depend_libs(self):
        depend_libs = ['cache.py', 'storage.py']
        _libs_dir = os.path.join(self.src_dir, ".." ,'libs')
        for file_name in depend_libs:
            _src_code_path = os.path.join(_libs_dir, file_name)
            if self.check(_src_code_path):
                continue
            src_code = open(_src_code_path, 'r').read()
            mod_name = file_name
            msg = int.to_bytes(len(mod_name), 1, 'little')
            msg += mod_name.encode('utf8')
            msg += int.to_bytes(0, 1, 'little') # source code
            msg += src_code.encode('utf8')

            print('++++++++++++++++deply:', file_name)
            r = eosapi.push_message('kitties','deploy',msg,{'kitties':'active'},rawargs=True)
            assert r
        producer.produce_block()

    def deploy_all(self):
        self.deploy_depend_libs()
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
        if self.check(_abs_file_path):
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
        r = eosapi.push_message('kitties','deploy',msg,{'kitties':'active'},rawargs=True)
        assert r

        producer.produce_block()
        self.save()
        time.sleep(0.2)

    def deploy_all_mpy(self):
        self.deploy_depend_libs()
        with producer:
            files = os.listdir(self.src_dir)
            for file_name in files:
                if not file_name.endswith('.py'):
                    continue
                if file_name in self.ignore_files:
                    continue
                self.deploy_mpy(file_name)
    def clean(self):
        self.last_sync = {}
        self.save()
        print('clean done')

sync = Sync(_ignore = ('main.py', 'test.py', 'ustruct.py', 'eoslib.py'))

@init
def deploy_depend_libs():
    sync.deploy_depend_libs()

@init
def deploy_all():
    sync.deploy_all()

@init
def deploy(src_file):
    sync.deploy(src_file)

@init
def deploy_all_mpy():
    sync.deploy_all_mpy()

@init
def deploy_mpy(file_name):
    sync.deploy_mpy(file_name)

def clean():
    sync.clean()

@init
def test(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('kitties','sayhello',name,{'kitties':'active'},rawargs=True)
        assert r

@init
def test2(count):
    import time
    import json
    contracts = []
    functions = []
    args = []
    per = []
    for i in range(count):
        functions.append('call')
        arg = str(i)
        args.append(arg)
        contracts.append('kitties')
        per.append({'kitties':'active'})

    ret = eosapi.push_messages(contracts, functions, args, per, True, rawargs=True)
    assert ret
    cost = ret['cost_time']
    eosapi.produce_block()
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

