import os
import initeos
import eosapi
import pickle

producer = eosapi.Producer()
CODE_TYPE_WAST = 0
CODE_TYPE_PY = 1
CODE_TYPE_MPY = 2

def init_(name, src, abi, curr_path, code_type=CODE_TYPE_MPY):
    _src_dir = os.path.dirname(os.path.abspath(curr_path))

    if src.find('/') < 0:
        src = os.path.join(_src_dir, src)

    if abi.find('/') < 0:
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
    if old_code:
        old_code = old_code[0]
        with open(src, 'rb') as f:
            code = f.read()
        if code_type == CODE_TYPE_WAST:
            code = eosapi.wast2wasm(code)
            old_code = eosapi.wast2wasm(old_code)
            if code == old_code:
                return
        elif (code == old_code[1:] or code == old_code):
            return

    with producer:
        if code_type == 0:
            r = eosapi.set_contract(name, src, abi, 0)
        else:
            r = eosapi.set_contract(name, src, abi, 1)
        assert r

