from eoslib import *
def apply(name, type):
    if type == N('sayhello'):
        print('sync hello')
        require_auth(N('async'))
        msg = read_action()
        print(msg.decode('utf8'))
