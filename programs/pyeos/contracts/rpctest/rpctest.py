from eoslib import *

def sayHello():
    print('hello,world')

def apply(name, type):
    print(name, type, N('sayhello'))
    if type == N('sayhello'):
#        print('read_action:', read_action())
        sayHello()
