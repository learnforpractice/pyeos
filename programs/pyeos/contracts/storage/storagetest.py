from backyard.storage import SDict
from eoslib import *

code = N('storagetest')

def apply(receiver, code, action):
    if action == N('sayhello'):
        a = SDict(code, N('a'))
        msg = read_action()
        a[msg] = msg
        return
        a1 = a[100]
        a2 = a[101]
        a3 = a[102]
#        print(a1, a2, a3)
        a[100] = 'hello1'
        a[101] = 'hello2'
        a[102] = 'hello3'
