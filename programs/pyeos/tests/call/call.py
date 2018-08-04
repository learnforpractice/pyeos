from eoslib import *
def apply(receiver, code, action):
    call_set_args(b'helloooooooooo')
    print(call_get_args())

    if action == N('sayhello'):
        require_auth(N('call'))
        msg = read_action()
        print(msg.decode('utf8'))
