import ustruct
from eoslib import *
code = N('kitties')

def deploy(mod_name, src_code):
    print('++++++++++++deploy:mod_name', mod_name)
    id = hash64(mod_name)
    itr = db_find_i64(code, code, code, id)
    if itr < 0:
        db_store_i64(code, code, code, id, src_code)
    else:
        db_update_i64(itr, code, src_code)

def apply(name, action):
    if action == N('deploy'):
        require_auth(code)
        msg = read_action()
        length = int.from_bytes(msg[:1], 'little')
        mod_name = msg[1:1+length]
        src_code = msg[1+length:]
        print('+++++++++++++++++src_code type:', src_code[0])
        deploy(mod_name, src_code)
    elif action == N('call'):
#        print('++++++++++++call')
        from kittycore import KittyCore
        core = KittyCore()
    elif action == N('del'):
        mod_name = read_action()
        id = hash64(mod_name)
        itr = db_find_i64(code, code, code, id)
        if itr >= 0:
            db_remove_i64(itr)
    else:
        pass
    
