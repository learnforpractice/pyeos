#import eoslib

def init():
    print('hello,world')

def sayHello():
    print('++++++++++++hello,world')
#        print('got message on chain:', eoslib.read_message())

def apply(name, type):
    print('hello,world', name, type)
#    if type == eoslib.N('sayhello'):
#        sayHello()
