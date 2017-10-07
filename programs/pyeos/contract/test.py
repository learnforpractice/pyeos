if __name__ == '__main__':
    import eoslib_dummy as eoslib
else:
    import eoslib
import struct

first = eoslib.N('b')
second = eoslib.N('c')
third = eoslib.N('q')
#third = eoslib.N('1')
def store():
    keys = struct.pack('QQQ',first,second,third)
    r = eoslib.store('test','test','account',keys,2,b'ggg')
    print(r)
    
def load():
    keys = struct.pack('QQQ',first,second,third)
    r = eoslib.load('test','test','account',keys,2,2)
    print(r)

def update():
    keys = struct.pack('QQQ',first,second,third)
    r = eoslib.update('test','test','account',keys,2,b'ggg')
    print(r)

def front():
    keys = struct.pack('QQQ',first,second,third)
    r = eoslib.front('test','test','account',keys,2,2)
    print(r)

def back():
    keys = struct.pack('QQQ',first,second,third)
    r = eoslib.back('test','test','account',keys,2,0)
    print(r)

def remove():
    keys = struct.pack('QQQ',first,second,third)
    r = eoslib.remove('test','test','account',keys,2,b'def')
    print(r)

def init():
    first = eoslib.N('b')
    second = eoslib.N('c')
    third = eoslib.N('a')
    keys = struct.pack('QQQ',first,second,third)
    r = eoslib.store('test','test','account',keys,2,b'bca')

    first = eoslib.N('x')
    second = eoslib.N('c')
    third = eoslib.N('c')
    keys = struct.pack('QQQ',first,second,third)
    r = eoslib.store('test','test','account',keys,2,b'xcb')


    first = eoslib.N('a')
    second = eoslib.N('c')
    third = eoslib.N('q')
    r = eoslib.front('test','test','account',keys,2,0)
    print('front',r)
    r = eoslib.back('test','test','account',keys,2,0)
    print('back',r)


def apply(name,type):
#    print('hello from python apply',name,type)
#    print(eoslib.n2s(name),eoslib.n2s(type))
    if type == eoslib.N(b'transfer'):
        pass

if __name__ == '__main__':
    init()
    apply(eoslib.N(b'python'),eoslib.N(b'transfer'))



