import db
from eoslib import N, read_action

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
        sayHello()
    elif action == N('play'):
        play()

