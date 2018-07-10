import ustruct
from eoslib import *
import db

def deploy(mod_name, src_code):
    code = N('kitties')
    print('++++++++++++deploy:mod_name', mod_name)
    id = hash64(mod_name)
    itr = db.find_i64(code, code, code, id)
    if itr < 0:
        db.store_i64(code, code, code, id, src_code)
    else:
        db.update_i64(itr, code, src_code)

    if mod_name.endswith('.mpy'):
        #try to remove py source to prevent micropython loading the wrong source
        mod_name = mod_name[:-3] + 'py'
        id = hash64(mod_name)
        itr = db.find_i64(code, code, code, id)
        if itr >= 0:
            db.remove_i64(itr)

def apply(receiver, code, action):
    if action == N('deploy'):
        require_auth(code)
        msg = read_action()
        length = int.from_bytes(msg[:1], 'little')
        mod_name = msg[1:1+length]
        src_code = msg[1+length:]
        print('+++++++++++++++++src_code type:', src_code[0])
        deploy(mod_name, src_code)
    elif action == N('call'):
        print('++++++++++++call')
        from kittycore import KittyCore
        core = KittyCore()
    elif action == N('sayhello'):
        print('hello,world')
    else:
        pass

