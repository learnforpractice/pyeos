from eoslib import store_str, load_str, N

def init():
    print('hello,world')

def sayHello():
    n = 0
    for i in range(1000):
        n += i
        n += i
#    print('++++++++++++hello,world', n)
#        print('got message on chain:', eoslib.read_message())

def apply(name, type):
    print('hello,world', name, type)
#    store_str(N('hello'), N('table'), 'helo', 'world')
    v = load_str(N('hello'), N('hello'), N('table'), 'helo')
    print('++++++++++++:', v)

    if type == eoslib.N('sayhello'):
        sayHello()

sayHello()

