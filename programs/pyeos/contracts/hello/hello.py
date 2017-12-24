import eoslib

def init():
    print('hello,world')

def sayHello():
        print('got message on chain:', eoslib.read_message())

def apply(name, type):
    if type == eoslib.N('sayhello'):
        sayHello()
