import db
from eoslib import *

def deploy(scope, table, id, data):
    code = N('vmstore')
    payer = code
    print('++++++++++++deploy:scope', n2s(scope), n2s(table))
    itr = db.find_i64(code, scope, table, id)
    if itr < 0:
        db.store_i64(scope, table, payer, id, data)
    else:
        db.update_i64(itr, payer, data)

def apply(receiver, code, action):
    print(n2s(code), n2s(action))
    if action == N('deploy'):
        require_auth(code)
        msg = read_action()
        print('++++++len(msg):', len(msg))
        scope = int.from_bytes(msg[:8], 'little')
        table = int.from_bytes(msg[8:16], 'little')
        id = int.from_bytes(msg[16:24], 'little')
        data = msg[24:]
        deploy(scope, table, id, data)
    elif action == N('delete'):
        require_auth(code)
        vm_name = read_action()
        vm_name = int.from_bytes(vm_name, 'little')
        itr = db.find_i64(code, code, code, vm_name)
        print('+++++++++++:', itr)
        if itr >= 0:
            db.remove_i64(itr)
        while True:
            itr = db.end_i64(code, code, vm_name)
            print('+++++++++++ end_i64 :', itr)
            if itr == -1:
                break
            itr, primary = db.previous_i64(itr)
            print('+++++++++++ previous_i64 :', itr)
            db.remove_i64(itr)

