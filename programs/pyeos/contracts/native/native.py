from eoslib import *
def deploy(account, src_code):
    code = N('native')
    print('++++++++++++deploy:account', account)
    itr = db_find_i64(code, code, code, account)
    if itr < 0:
        db_store_i64(code, code, code, account, src_code)
    else:
        db_update_i64(itr, code, src_code)

def apply(receiver, code, action):
    print(n2s(code), n2s(action))
    if action == N('deploy'):
        require_auth(code)
        msg = read_action()
        print('++++++len(msg):', len(msg))
        account = int.from_bytes(msg[:8], 'little')
        src_code = msg[8:]
        deploy(account, src_code)
