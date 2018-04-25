from eoslib import *
code = N('lab')
def apply(name, type):
    if type == N('sayhello'):
        require_auth(code)
        msg = read_action()
        print(msg.decode('utf8'))
