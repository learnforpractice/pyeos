from eoslib import *
def apply(name, type):
    if type == N('sayhello'):
        print('hello')
        require_auth(N('lockunlock'))
        msg = read_action()
        print(msg.decode('utf8'))
 