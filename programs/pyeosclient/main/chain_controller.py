import imp
import struct
import eosapi
import eostypes
from eosapi import N
import blockchain

import logging as log

code_map = {}

def load(name, code):
    global code_map
    ret = 0
#    print('python_load:',name)
    module = code_map.get(name)

    if not module or (module.__code != code):
        try:
            new_module = imp.new_module(str(name))
            exec(code,vars(new_module))
            code_map[name] = new_module
            new_module.__code = code
        except Exception as e:
            log.exception(e)
            ret = -1
    return ret;

def call(name, func, args):
    global code_map
    ret = -1
#    print('python_call:',func)
    try:
        module = code_map[name]
        func = getattr(module,func)
        func(*args)
        ret = 0
    except Exception as e:
        log.exception(e)
    return ret

class chain_controller(object):
    def __init__(self):
        pass

    def process_transaction(self,trx):
        for msg in trx.get_messages():
            code_info = eosapi.get_code(msg.code)
            if code_info:
                ctx = blockchain.apply_context(msg.code, trx, msg)
                blockchain.apply_message(ctx)
                
                '''
                if code_info[-1] == 0:
                    pass
                elif code_info[-1] == 1:
                    ctx = blockchain.apply_context(msg.code, trx, msg)
                    blockchain.set_current_context(ctx)
                    load(msg.code, code_info[0])
                    call(msg.code, 'apply', [N(b'currency'), N(b'transfer')])
                '''
