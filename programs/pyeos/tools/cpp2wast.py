#./pyeos/pyeos --manual_gen_block --debug -i
import os
import time
import struct

import wallet
import eosapi
from eosapi import N

import initeos
import subprocess
import  shlex

src_path = os.path.dirname(os.path.abspath(__file__))

def set_src_path(_path):
    global src_path
    src_path = _path

def need_build(src_file):
    full_src_file_path = os.path.join(src_path, src_file)

    wast_file = src_file.replace('.cpp', '.wast')
    full_wast_file_path = os.path.join(src_path, wast_file)

    t1 = os.path.getmtime(full_src_file_path)
    t2 = os.path.getmtime(full_wast_file_path)
    if t1 > t2:
        return True
    return False

def build(src_file = 'code.cpp', force=False):
    if not force:
        if not need_build(src_file):
            return True

    cpp_bc_file = src_file + '.bc'
#currency.cpp.bc
    full_src_file_path = os.path.join(src_path, src_file)
    full_cpp_bc_file_path = os.path.join(src_path, cpp_bc_file)

    clang_cmd = "/usr/local/wasm/bin/clang;-emit-llvm;-O3;--std=c++14;--target=wasm32;-ffreestanding;-nostdlib;-nostdlibinc;-fno-threadsafe-statics;-fno-rtti;-fno-exceptions;-c;{0};-o;{1};-Weverything;-Wno-c++98-compat;-Wno-old-style-cast;-Wno-vla;-Wno-vla-extension;-Wno-c++98-compat-pedantic;-Wno-missing-prototypes;-Wno-missing-variable-declarations;-Wno-packed;-Wno-padded;-Wno-c99-extensions;-Wno-documentation-unknown-command;-I;/Users/newworld/dev/pyeos/contracts;-I;/Users/newworld/dev/pyeos/externals/magic_get/include;-isystem;/Users/newworld/dev/pyeos/contracts/libc++/upstream/include;-isystem;/Users/newworld/dev/pyeos/contracts/musl/upstream/include;-isystem;/Users/newworld/.hunter/_Base/a882971/f6bfdc3/786a69d/Install/include;-isystem;/Users/newworld/dev/pyeos/contracts/libc++/upstream/include;-isystem;/Users/newworld/dev/pyeos/contracts/musl/upstream/include;-isystem;/Users/newworld/.hunter/_Base/a882971/f6bfdc3/786a69d/Install/include"
    clang_cmd = clang_cmd.replace(';', ' ')
    cmds = shlex.split(clang_cmd.format(full_src_file_path, full_cpp_bc_file_path))
#    print(cmds)
    ret = subprocess.call(cmds)
    print('compile ', ret)
    if ret:
        return False

#currency.bc currency.cpp.bc
    bc_file = src_file.replace('.cpp', '.bc')
    full_bc_file_path = os.path.join(src_path, bc_file)

    link_cmd = '/usr/local/wasm/bin/llvm-link -only-needed -o {0} {1} /Users/newworld/dev/pyeos/build/contracts/libc++/libc++.bc;/Users/newworld/dev/pyeos/build/contracts/musl/libc.bc;/Users/newworld/dev/pyeos/build/contracts/eosiolib/eosiolib.bc'
    link_cmd = link_cmd.replace(';', ' ')
    cmds = shlex.split(link_cmd.format(full_bc_file_path, full_cpp_bc_file_path))
    ret = subprocess.call(cmds)
    print('link', ret)
    if ret:
        return False

    s_file = src_file.replace('.cpp', '.s')
    full_s_file_path = os.path.join(src_path, s_file)

    llc_cmd = "/usr/local/wasm/bin/llc -thread-model=single -asm-verbose=false -o {0} {1}".format(full_s_file_path, full_bc_file_path)
    cmds = shlex.split(llc_cmd)
    ret = subprocess.call(cmds)
    print('llc', ret)
    if ret:
        return False

    wast_file = src_file.replace('.cpp', '.wast')
    full_wast_file_path = os.path.join(src_path, wast_file)

    s2wasm_cmd = '/Users/newworld/dev/pyeos/build/externals/binaryen/bin/eosio-s2wasm -o {0} -s 4096  {1}'.format(full_wast_file_path, full_s_file_path)
    cmds = shlex.split(s2wasm_cmd)
    ret = subprocess.call(cmds)
    print('s2wasm', ret)
    if ret:
        return False

    return True


