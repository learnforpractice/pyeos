import os
import json
import math

import rodb
import debug
import eosapi
import wallet
import initeos
from common import prepare
from eosapi import N
import unittest

test_code = '''
function test_memory()
    a = {}
    for i=1, 1024*64 do
        a[i] = i
    end
end

function apply(receiver, account, act)
    if act == N('sayhello') then
        test_memory()
    end
    return 1
end

'''

def deploy(account, src):
    if not eosapi.get_account(account):
        print('account does not exists, create it.')
        r = eosapi.create_account2('eosio', account, initeos.key1, initeos.key2)
        assert r

    '''
    abs_src_file = os.path.dirname(__file__)
    last_update = eosapi.get_code_update_time_ms(account)
    modify_time = os.path.getmtime(abs_src_file)*1000
    if last_update >= modify_time:
        return
    '''

    actions = []
    _src_dir = os.path.dirname(__file__)
    abi_file = os.path.join(_src_dir, 'tester.abi')
    setabi = eosapi.pack_setabi(abi_file, eosapi.N(account))
    act = ['eosio', 'setabi', setabi, {account:'active'}]
    actions.append(act)

    args = eosapi.pack_args("eosio", 'setcode', {'account':account,'vmtype':10, 'vmversion':0, 'code':src.encode('utf8').hex()})
    act = ['eosio', 'setcode', args, {account:'active'}]
    actions.append(act)

    r, cost = eosapi.push_actions(actions)
    print(r['except'])
    print(r['elapsed'])

def test_memory():
    deploy('luatest', test_code)
    r = eosapi.push_action('luatest', 'sayhello', '', {'luatest':'active'})
    print(r)

def test_io():
    test_code = '''
    require 'io'
    function apply(receiver, account, act)
        if act == N('sayhello') then
            require 'io'
            print(io)
            print('hello, lua world')
        end
        return 1
    end
    '''
    deploy('luatest', test_code)
    r = eosapi.push_action('luatest', 'sayhello', '', {'luatest':'active'})
    print(r)

def test_string_find():
#http://lua-users.org/lists/lua-l/2011-02/msg01595.html
    test_code = '''
    require 'string'
    function apply(receiver, account, act)
        if act == N('sayhello') then
            string.find(string.rep("a", 50), string.rep("a?", 50)..string.rep("a", 50))
        end
        return 1
    end
    '''
    deploy('luatest', test_code)
    r = eosapi.push_action('luatest', 'sayhello', '', {'luatest':'active'})
    print(r)

cfg ='''disabled_modules = {io = 1, os=1}
memory_limit = 1024*64
output_limit = 1024
log_level = 7
remove_entries = {
  os = {"getenv", "execute"},
  string = {"dump"}
}
'''
script = '''
require 'io'
a = {}
for i=0, 1024*10 do
    a[i] = i
--    print(i)
end

print('hello,world')

'''

script = '''
--    print('hello,world')
    f = loadstring("print('hello,world')")
    print(f())
    print('hello,world')
'''

script = '''
require 'io'
require 'os'
print('os:', os)
print(io.write('abc'))
'''

def test_run_script():
    debug.vm_run_lua_script(cfg, script)

class LUATestCase(unittest.TestCase):
    def setUp(self):
        pass

    @unittest.expectedFailure
    def test_memory(self):
        test_memory()

    @unittest.expectedFailure
    def test_string_find(self):
        test_string_find()


    def tearDown(self):
        pass

def ut():
    unittest.main(module=ut.__module__, exit=False)
