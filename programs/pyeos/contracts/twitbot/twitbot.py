from eoslib import *
def apply(name, type):
    if type == N('sayhello'):
        require_auth(N('twitbot'))
        msg = read_action()
        print(msg.decode('utf8'))
