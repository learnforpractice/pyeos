from eoslib import *

code = N('actiontest')
id = N('name')

def sayHello():

#do some math
    if 0:
        n = 0
        for i in range(1000):
            n += i
        print(n)

    name = read_action()
#    print('hello', name)

    itr = db_find_i64(code, code, code, id)
    if itr >= 0: # value exist, update it
        old_name = db_get_i64(itr)
#        print('hello,', old_name)
        db_update_i64(itr, code, name)
    else:
        pass
        db_store_i64(code, code, code, id, name)

def apply(name, type):
    if type == N('sayhello'):
        sayHello()


