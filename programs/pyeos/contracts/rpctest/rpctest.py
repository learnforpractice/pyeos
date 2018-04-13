from eoslib import *

def sayHello():
    n = N('rpctest')
    id = N('name')

    name = read_action()
    print('hello', name)

    itr = db_find_i64(n, n, n, id)
    if itr >= 0: # value exist, update it
        old_name = db_get_i64(itr)
        print('hello,', old_name)
        db_update_i64(itr, n, name)
    else:
        db_store_i64(n, n, n, id, name)

def apply(name, type):
    print('apply:', name, type)
    if type == N('sayhello'):
#        print('read_action:', read_action())
        sayHello()
