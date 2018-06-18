from eoslib import *
from backyard import storage


def apply(receiver, code, action):
    code = N('storagetest')
    if action == N('sayhello'):
        a = storage.SDict(code, N('a'))
        msg = read_action()
        a[msg] = msg
        try:
            a1 = a[100]
            a2 = a[101]
            a3 = a[102]
#            print(a1, a2, a3)
        except Exception as e:
            print(e)
#        print(a1, a2, a3)
        a[100] = 'hello1'
        a[101] = 'hello2'
        a[102] = 'hello3'
