import db
from eoslib import N, read_action
import logging

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
        print('helloooooooo,', old_name)
        db.update_i64(itr, payer, name)
    else:
        db.store_i64(scope, table, payer, id, name)

def apply(receiver, code, action):
    print("+++++++++++++++hello from cython")
    if action == N('sayhello'):
        sayHello()
    elif action == N('play'):
        play()
