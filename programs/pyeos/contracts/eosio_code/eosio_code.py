import db
import struct
import eoslib
from eoslib import N, read_action, send_inline, transfer_inline

#a = bytes(1024*1024*1024)

def sayHello():
    n = N('hello')
    id = N('name')

    name = read_action()
    print('hello', name)
    code = n
    scope = n
    table = n
    payer = n
    itr = db.find_i64(code, scope, table, id)
    if itr >= 0: # value exist, update it
        old_name = db.get_i64(itr)
        print('hello,', old_name)
        db.update_i64(itr, payer, name)
    else:
        db.store_i64(scope, table, payer, id, name)

def play():
    from backyard import garden
    garden.play()

def apply(receiver, code, action):
    if action == N('sayhello'):
        from hello import sayhello
        sayhello.sayHello('mike')
    elif action == N('setcode'):
        data = read_action()
        code_account, code_name, code_type = struct.unpack('QQB', data[:17])
        src_code = eoslib.unpack_bytes(data[17:])
        eoslib.set_code_ext(code_account, 1, code_name, src_code)
        
    elif action == N('import'):
        from eosio.code import hello

