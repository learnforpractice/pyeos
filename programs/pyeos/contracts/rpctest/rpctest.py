from eoslib import *
code = N('rpctest')

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

def test():
    n = N('rpctest')
    id = N('name')

    name = read_action()
    print('hello', name)

    itr = db_find_i64(n, n, n, id)
    if itr >= 0: # value exist, update it
        old_name = db_get_i64(itr)
        print('hello,', old_name)
    else:
        print('not found!')

def apply(receiver, code, action):
    if action == N('sayhello'):
        msg = read_action()
        key = N('hellooo')

        itr = db_end_i64(code, code, code)
        print('end: ', itr)
        while itr != -1:
            itr, primary = db_previous_i64(itr)
            if itr < 0:
                break
            print('previous: ', itr, n2s(primary), db_get_i64(itr))

        itr = db_find_i64(code, code, code, key)
        print('find 1: ', itr)
    
        while itr >= 0:
            db_remove_i64(itr)
            itr = db_find_i64(code, code, code, key)
            print('find 2:', itr)

        itr = db_end_i64(code, code, code)
        print('end: ', itr)
        while itr != -1:
            itr, primary = db_previous_i64(itr)
            if itr < 0:
                break
            print('previous: ', itr, n2s(primary), db_get_i64(itr))

        itr = db_store_i64(code, code, code, key, msg)
        print('store: ', itr)
        if itr != -1:
            ret = db_get_i64(itr)
        print('ret: ', ret)
        itr = db_end_i64(code, code, code)
        print('end: ', itr)

        while itr != -1:
            itr, primary = db_previous_i64(itr)
            if itr < 0:
                break
            print('previous: ', itr, n2s(primary), db_get_i64(itr))

#        print('read_action:', read_action())
#]        sayHello()
