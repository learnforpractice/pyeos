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
from tools import tools_config

#src_path = os.path.dirname(os.path.abspath(__file__))
src_path = '../../programs/pyeos/contracts/lab'

parser = OptionParser()
parser.add_option("--clang",            dest="clang",           default=tools_config.clang,          help="")
parser.add_option("--link",             dest="link",            default=tools_config.link,           help="")
parser.add_option("--llc",              dest="llc",             default=tools_config.llc,            help="")
parser.add_option("--s2wast",           dest="s2wast",          default=tools_config.s2wast,         help="")
parser.add_option("--abigen",            dest="abigen",         default=tools_config.abigen,          help="")
parser.add_option("--src-path",         dest="src_path",        default='.',       help="Folder to locate source code to compile")


(options, args) = parser.parse_args()

clang           = options.clang
link            = options.link
llc             = options.llc
s2wast          = options.s2wast
src_path        = options.src_path

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

    target = os.path.join(src_path, src_file[:-4])

    clang_cmd = tools_config.clang
    cmds = shlex.split(clang_cmd.format(target=target))
    print(cmds)
    ret = subprocess.call(cmds)
    print('compile ', ret)
    if ret:
        return False

    link_cmd = tools_config.link
    cmds = shlex.split(link_cmd.format(target=target))
    print(cmds)
    ret = subprocess.call(cmds)
    print('link', ret)
    if ret:
        return False


    llc_cmd = tools_config.llc.format(target=target)
    cmds = shlex.split(llc_cmd)
    print(cmds)
    ret = subprocess.call(cmds)
    print('llc', ret)
    if ret:
        return False

    s2wasm_cmd = '{s2wast} -o {target}.wast -s 4096  {target}.s'.format(target=target,s2wast=tools_config.s2wast)
    cmds = shlex.split(s2wasm_cmd)
    print(cmds)

    ret = subprocess.call(cmds)
    print('s2wasm', ret)
    if ret:
        return False

    '''
    abigen_cmd = tools_config.abigen.format(target=target, src_dir=src_path)
    cmds = shlex.split(abigen_cmd)
    print(cmds)
    ret = subprocess.call(cmds)
    print('abigen', ret)
    if ret:
        return False
    '''


    return True


def build_native(src_file, lib_name, force=False):
    INCLUDES = "-I{root_src_dir}/contracts/eosio.token/.. \
    -I{root_src_dir}/externals/magic_get/include \
    -I{root_src_dir}/contracts \
    -I{root_src_dir}/libraries/chain/include \
    -I{root_src_dir}/libraries/fc/include \
    -I{root_src_dir}/libraries/softfloat/source/include \
    -I{root_src_dir}/libraries/softfloat/source/8086-SSE \
    -I{root_src_dir}/libraries/softfloat/build/Linux-x86_64-GCC \
    -I{src_path}"

    INCLUDES = INCLUDES.format(root_src_dir=tools_config.root_src_dir, src_path=src_path)
    compile_cmd = 'clang++  -Deosio_token_native_EXPORTS {includes} -Wall -Wno-deprecated-declarations -DDEBUG -g -fPIC   -std=gnu++14 -o {src_file}.o -c {src_path}/{src_file}'
    compile_cmd = compile_cmd.format(includes=INCLUDES, src_path=src_path, src_file=src_file)

    compile_cmd = shlex.split(compile_cmd)
    print(compile_cmd)

    ret = subprocess.call(compile_cmd)
    print('compile_cmd', ret)
    if ret:
        return False

    link_cmd = 'clang++  -Wall -Wno-deprecated-declarations -DDEBUG -g -dynamiclib -Wl,-headerpad_max_install_names  -o {src_path}/lib{lib_name}.dylib -install_name @rpath/lib{lib_name}.dylib {src_name}.o -Wl,-rpath,{build_dir}/contracts/eosiolib_native -Wl,-rpath,{build_dir}/libraries/softfloat {build_dir}/contracts/eosiolib_native/libeosiolib_natived.dylib {build_dir}/libraries/softfloat/libsoftfloatd.dylib'
    link_cmd = link_cmd.format(lib_name=lib_name, src_name=src_file, build_dir=tools_config.build_dir, src_path=src_path)
    link_cmd = shlex.split(link_cmd)
    ret = subprocess.call(link_cmd)
    print('link_cmd', ret)
    if ret:
        return False

