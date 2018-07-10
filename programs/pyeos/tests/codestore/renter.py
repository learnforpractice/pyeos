from eoslib import *
def apply(receiver, code, action):
    if action == N('sayhello'):
        require_auth(N('code'))
        from codestore import math
        ret = math.add(1, 2)
        print('1+2 =', ret)
