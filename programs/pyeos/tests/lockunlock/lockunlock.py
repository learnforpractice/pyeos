from eoslib import *
def apply(receiver, code, action):
    if action == N('sayhello'):
        print('hello')
        require_auth(N('lockunlock'))
        msg = read_action()
        print(msg.decode('utf8'))
 