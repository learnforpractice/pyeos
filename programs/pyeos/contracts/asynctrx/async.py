from eoslib import *
def apply(name, type):
    if type == N('sayhello'):
        print('hello,world')
        require_auth(N('async'))
        msg = read_action()
        print(msg.decode('utf8'))
