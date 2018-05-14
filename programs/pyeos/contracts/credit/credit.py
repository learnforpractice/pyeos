from eoslib import *

def apply(receiver, code, action):
    require_auth(N('credit'))
    msg = read_action()
    account = int.from_bytes(msg, 'little')
    eosio_assert(is_account(account), "not an account")
    if action == N('addtowl'):
        itr = db_find_i64(code, code, code, account)
        if itr >=0:
            res = db_get_i64(itr)
            eosio_assert(res != b'1', 'account already in whitelist')
            db_update_i64(itr, code, b'1')
        else:
            db_store_i64(code, code, code, account, b'1')
    elif action == N('rmfromwl'):
        itr = db_find_i64(code, code, code, account)
        eosio_assert(itr >= 0, 'account not found')
        res = db_get_i64(itr)
        eosio_assert(res == b'1', 'account not in whitelist')
        db_remove_i64(itr)
    elif action == N('addtobl'):
        itr = db_find_i64(code, code, code, account)
        if itr >=0:
            res = db_get_i64(itr)
            eosio_assert(res != b'2', 'account already in blacklist')
            db_update_i64(itr, code, b'2')
        else:
            db_store_i64(code, code, code, account, b'2')
    elif action == N('rmfrombl'):
        itr = db_find_i64(code, code, code, account)
        eosio_assert(itr >= 0, 'account not found')
        res = db_get_i64(itr)
        eosio_assert(res == b'2', 'account not in blacklist')
        db_remove_i64(itr)
    else:
        eosio_assert(0, 'action  is not supported')