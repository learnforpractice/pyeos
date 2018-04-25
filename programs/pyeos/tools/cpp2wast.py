import os
import sys
import time
import shlex
import struct
import subprocess
from optparse import OptionParser

import wallet
import eosapi
import initeos
from eosapi import N


#src_path = os.path.dirname(os.path.abspath(__file__))
src_path = '../../programs/pyeos/contracts/lab'

src_root_path = '/Users/newworld/dev/pyeos'
boost_path = '/Users/newworld/.hunter/_Base/a882971/f6bfdc3/786a69d/Install/include'

clang = '/usr/local/wasm/bin/clang'
link = '/usr/local/wasm/bin/llvm-link'

llc = '/usr/local/wasm/bin/llc'
s2wast = '/Users/newworld/dev/pyeos/build/externals/binaryen/bin/eosio-s2wasm'

parser = OptionParser()
parser.add_option("--clang",            dest="clang",           default=clang,          help="")
parser.add_option("--link",             dest="link",            default=link,           help="")
parser.add_option("--llc",              dest="llc",             default=llc,            help="")
parser.add_option("--s2wast",           dest="s2wast",          default=s2wast,         help="")
parser.add_option("--src-path",         dest="src_path",        default=src_path,       help="Folder to locate source code to compile")
parser.add_option("--src-root-path",    dest="src_root_path",   default=src_root_path,  help="Eos source code root path")
parser.add_option("--boost-path",       dest="boost_path",      default=boost_path,     help="Boost installed path")


(options, args) = parser.parse_args()

clang           = options.clang
link            = options.link
llc             = options.llc
s2wast          = options.s2wast
src_path        = options.src_path
src_root_path   = options.src_root_path
boost_path      = options.boost_path

def set_src_path(_path):
    global src_path
    src_path = _path

def need_build(src_file):
    full_src_file_path = os.path.join(src_path, src_file)

    wast_file = src_file.replace('.cpp', '.wast')
    full_wast_file_path = os.path.join(src_path, wast_file)

    if not os.path.exists(full_wast_file_path):
        return True

    t1 = os.path.getmtime(full_src_file_path)
    t2 = os.path.getmtime(full_wast_file_path)
    if t1 > t2:
        return True
    return False

def build(src_file = 'lab.cpp', force=False):
    if not force:
        if not need_build(src_file):
            return True

    cpp_bc_file = src_file + '.bc'
#currency.cpp.bc
    full_src_file_path = os.path.join(src_path, src_file)
    full_cpp_bc_file_path = os.path.join(src_path, cpp_bc_file)

    clang_cmd = "{clang};-emit-llvm;-O3;--std=c++14;--target=wasm32;-ffreestanding;-nostdlib;-nostdlibinc;-fno-threadsafe-statics;-fno-rtti;-fno-exceptions;-c;{0};-o;{1};-Weverything;-Wno-c++98-compat;-Wno-old-style-cast;-Wno-vla;-Wno-vla-extension;-Wno-c++98-compat-pedantic;-Wno-missing-prototypes;-Wno-missing-variable-declarations;-Wno-packed;-Wno-padded;-Wno-c99-extensions;-Wno-documentation-unknown-command;-I;{2}/contracts;-I;{2}/externals/magic_get/include;-isystem;{2}/contracts/libc++/upstream/include;-isystem;{2}/contracts/musl/upstream/include;-isystem;{2}/contracts/libc++/upstream/include;-isystem;{2}/contracts/musl/upstream/include;-isystem;{boost_path}"
    clang_cmd = clang_cmd.replace(';', ' ')
    cmds = shlex.split(clang_cmd.format(full_src_file_path, full_cpp_bc_file_path, src_root_path, clang=clang, boost_path=boost_path))
    print(cmds)
    ret = subprocess.call(cmds)
    print('compile ', ret)
    if ret:
        return False

#currency.bc currency.cpp.bc
    bc_file = src_file.replace('.cpp', '.bc')
    full_bc_file_path = os.path.join(src_path, bc_file)

    link_cmd = '{link} -only-needed -o {0} {1} {2}/build/contracts/libc++/libc++.bc;{2}/build/contracts/musl/libc.bc;{2}/build/contracts/eosiolib/eosiolib.bc'
    link_cmd = link_cmd.replace(';', ' ')
    cmds = shlex.split(link_cmd.format(full_bc_file_path, full_cpp_bc_file_path, src_root_path, link=link))
    print(cmds)
    ret = subprocess.call(cmds)
    print('link', ret)
    if ret:
        return False

    s_file = src_file.replace('.cpp', '.s')
    full_s_file_path = os.path.join(src_path, s_file)

    llc_cmd = "{llc} -thread-model=single -asm-verbose=false -o {0} {1}".format(full_s_file_path, full_bc_file_path, llc=llc)
    cmds = shlex.split(llc_cmd)
    print(cmds)
    ret = subprocess.call(cmds)
    print('llc', ret)
    if ret:
        return False

    wast_file = src_file.replace('.cpp', '.wast')
    full_wast_file_path = os.path.join(src_path, wast_file)

    s2wasm_cmd = '{s2wast} -o {0} -s 4096  {1}'.format(full_wast_file_path, full_s_file_path,s2wast=s2wast)
    cmds = shlex.split(s2wasm_cmd)
    print(cmds)

    ret = subprocess.call(cmds)
    print('s2wasm', ret)
    if ret:
        return False

    return True


