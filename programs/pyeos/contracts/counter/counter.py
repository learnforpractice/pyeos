from eoslib import *

def count():
    code = N('counter')
    counter_id = N('counter')

    itr = db_find_i64(code, code, code, counter_id)
    if itr >= 0: # value exists, update it
        counter = db_get_i64(itr)
        counter = int.from_bytes(counter, 'little')
        counter += 1
        counter = int.to_bytes(counter, 4, 'little')
        db_update_i64(itr, code, counter)
    else:
        counter = int.to_bytes(1, 4, 'little')
        db_store_i64(code, code, code, counter_id, counter)

def apply(receiver, code, action):
    if action == N('count'):
        count()

