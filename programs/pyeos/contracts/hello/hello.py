import eoslib

def init():
    print('hello,world')

def apply(name, type):
    print(eoslib.n2s(name), eoslib.n2s(type))
    if type == eoslib.N('sayhello'):
        eoslib.require_auth(eoslib.N('hello'))
        print('hello,world')
        print('read message:', eoslib.read_message())
