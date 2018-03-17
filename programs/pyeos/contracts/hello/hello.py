from eoslib import *

def init():
    print('hello,world')

def sayHello():
    n = 0
    for i in range(100):
        n += i
        n += i
#    print('++++++++++++hello,world', n)
#        print('got message on chain:', eoslib.read_message())

def apply(name, type):
    print('hello,world', name, type)
#    store_str(N('hello'), N('table'), 'helo', 'world')
    v = load_str(N('hello'), N('hello'), N('table'), 'helo')
    print('++++++++++++:', v)
    a = read_action()
    print('++++++++++++:', a, len(a))
    
    print('sender:', n2s(current_sender()))
    print('receiver:', n2s(current_receiver()))
    
    if type == N('sayhello'):
        sayHello()

sayHello()

