from eoslib import *
code = N('lab')
def apply(receiver, code, action):
    if action == N('sayhello'):
        require_auth(code)
        msg = read_action()
        print(msg.decode('utf8'))
