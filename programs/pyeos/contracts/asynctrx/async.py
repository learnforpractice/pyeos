from eoslib import *
def apply(receiver, code, action):
    if action == N('sayhello'):
        print('hello,world')
        require_auth(N('async'))
        msg = read_action()
        print(msg.decode('utf8'))
