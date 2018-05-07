#!/usr/bin/python3
import os
import sys
from optparse import OptionParser

test_py = \
'''import os
import time
import struct

import wallet
import eosapi
import initeos

from eosapi import N
from tools import cpp2wast

from common import prepare, producer

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init(func):
    def func_wrapper(wasm={1}, *args, **kwargs):
        if wasm:
            prepare('{0}', '{0}.wast', '{0}.abi', 0, __file__)
        else:
            prepare('{0}', '{0}.py', '{0}.abi', 2, __file__)
        return func(*args, **kwargs)
    return func_wrapper

@init
def test(msg='hello,world', wasm={1}):
    with producer:
        r = eosapi.push_message('{0}', 'sayhello', msg, {{'{0}':'active'}})
        assert r
'''
py_src = \
'''from eoslib import *
def apply(name, type):
    if type == N('sayhello'):
        require_auth(N('{0}'))
        msg = read_action()
        print(msg.decode('utf8'))
'''

cpp_src = \
'''#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
extern "C" {{
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {{
      auto self = receiver;
      if( code == self ) {{
         switch( action ) {{
            case N(sayhello):
                  size_t size = action_data_size();
                  if (size > 128) {{
                      size = 128;
                  }}
                  char msg[size+1];
                  msg[size] = '\\0';
                  read_action_data(msg, size);
                  prints(msg);
            break;
         }}
         eosio_exit(0);
      }}
   }}
}}
'''

abi_src = \
'''{
  "actions": [{
      "name": "sayhello",
      "type": "raw"
    }
  ]
}
'''

class Sketch(object):
    def __init__(self, _account, _project_dir = '.', _project_type='py', _force=False):
        self.account = _account
        if _project_dir[0] == '/':
            self.project_dir = _project_dir
        else:
            self.project_dir = os.path.join(os.getcwd(), _project_dir)

        self.project_type = _project_type
        self.force = _force
        self.mkdir(_project_dir)

    def build(self):
        self.build_source()
        self.build_abi()
        self.build_test()

    def build_source(self):
        if self.project_type == 'py':
            src_file = os.path.join(self.project_dir, self.account+'.py')
            if os.path.exists(src_file) and not self.force:
                print(f'{self.project_dir} already exists')
                raise Exception(f'{src_file} already exists')
            print('Creating source file  \t:', src_file)
            with open(src_file, 'w') as f:
                f.write(py_src.format(self.account))
        elif self.project_type == 'cpp':
            src_file = os.path.join(self.project_dir, self.account+'.cpp')
            print('Creating source file  \t:', src_file)
            with open(src_file, 'w') as f:
                f.write(cpp_src.format(self.account))
        else:
            raise Exception('language not supported: '+self.project_type)

    def build_abi(self):
        src_file = os.path.join(self.project_dir, self.account+'.abi')
        print('Creating abi file \t:', src_file)
        with open(src_file, 'w') as f:
            f.write(abi_src)

    def build_test(self):
        wasm = False
        if self.project_type == 'cpp':
            wasm = True
        src_file = os.path.join(self.project_dir, 't.py')
        print('Creating test file \t:', src_file)
        with open(src_file, 'w') as f:
            f.write(test_py.format(self.account, wasm))

    def mkdir(self, _path):
        _path = os.path.normpath(_path)
        __path = ''
        for _cur_dir in _path.split(os.sep):
            __path = os.path.join(__path, _cur_dir)
            if not os.path.exists(__path):
                print('Creating directory \t:', __path)
                os.mkdir(__path)

def build(account = 'hello', dir = 'helloworld', lang='py', force=False):
    s = Sketch(account, dir, lang, force)
    s.build()

if __name__ == '__main__':
    usage = "usage: python sketch.py [options]"
    parser = OptionParser(usage=usage)
    parser.add_option("--account",  dest="account", default='world',   help="Account name")
    parser.add_option("--lang",     dest="lang",    default='py',   help="Language type, 'py' or 'cpp'")
    parser.add_option("--dir",      dest="dir",     default='helloworld',    help="Project directory")
    parser.add_option("--init",     dest="init",    default=True,   action="store_true", help="Initialize project")

    (options, args) = parser.parse_args()
    print(options, args)
    s = Sketch(options.account, options.dir, options.lang)
    s.build()

