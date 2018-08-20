from eoslib import *
from backyard.token import transfer

code = N('codestore')

def deploy(mod_name, src_code):
    print('++++++++++++deploy:mod_name', mod_name)
    id = hash64(mod_name)
    itr = db_find_i64(code, code, code, id)
    if itr < 0:
        db_store_i64(code, code, code, id, src_code)
    else:
        db_update_i64(itr, code, src_code)

    mod_name = mod_name.decode('utf8')
    if mod_name.endswith('.mpy'):
        __import__('codestore.'+mod_name[:-4])
    elif mod_name.endswith('.py'):
        __import__('codestore.'+mod_name[:-3])

def apply(receiver, code, action):
    if action == N('sayhello'):
        require_auth(N('codestore'))
        msg = read_action()
        print(msg.decode('utf8'))
    elif action == N('deploy'):
        require_auth(code)
        msg = read_action()
        length = int.from_bytes(msg[:1], 'little')
        mod_name = msg[1:1+length]
        src_code = msg[1+length:]
#        print('+++++++++++++++++src_code type:', src_code[0])
#        print(src_code)
        deploy(mod_name, src_code)
    elif action == N('transfer'):
        msg = read_action()
        print('transfer', msg)
        t = transfer()
        t.unpack(msg)
        t.p()
        auction = SimpleAuction()
        auction.bid(t._from, t.amount)