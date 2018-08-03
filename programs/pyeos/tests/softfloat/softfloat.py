import db
from eoslib import N, read_action

def sayHello():
    print(0.1+0.1+0.1)
    print(pow(10.12, 2))

    print(1.0>2.0)
    print(1.0>=2.0)
    print(1.0<2.0)
    print(1.0<=2.0)
    print(1.0!=2.0)
    print(1.0==2.0)
    print(1.0==1.0)

    print(1.000000000001 == 1.000000000002)
    print(1.000000000001 < 1.000000000002)
    print(1.0000000000000000000000000000001 > 1.0)
    print(1.00000000001 > 1.0)

    print(8/3)

    print(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff/3)

def apply(receiver, code, action):
    if action == N('sayhello'):
        sayHello()
    elif action == N('play'):
        play()

