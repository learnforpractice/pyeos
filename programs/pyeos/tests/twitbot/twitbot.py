from eoslib import *
def apply(receiver, code, action):
    if action == N('sayhello'):
        require_auth(N('twitbot'))
        msg = read_action()
        print(msg.decode('utf8'))
