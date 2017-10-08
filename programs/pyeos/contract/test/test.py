if __name__ == '__main__':
    import eoslib_dummy as eoslib
    from eoslib.dummy import N
else:
    import eoslib
    from eoslib import N
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

def test_x64x64x64():
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
    keys = struct.pack('QQQ',first,second,third)
    r = eoslib.front('test','test','account',keys,2,0)
    print('front',r)
    r = eoslib.back('test','test','account',keys,2,0)
    print('back',r)

def test_x128x128():
    first = eoslib.N('b')
    second = eoslib.N('c')
    keys = struct.pack('QQQQ',first,0,second,0)
    r = eoslib.store('test','test','account',keys,1,b'bca')

    first = eoslib.N('a')
    second = eoslib.N('c')
    keys = struct.pack('QQQQ',first,0,second,0)
    r = eoslib.store('test','test','account',keys,1,b'cc')

    first = eoslib.N('1')
    second = eoslib.N('c')
    keys = struct.pack('QQQQ',first,0,second,0)
    r = eoslib.store('test','test','account',keys,1,b'1c')

    first = eoslib.N('2')
    second = eoslib.N('d')
    keys = struct.pack('QQQQ',first,0,second,0)
    r = eoslib.store('test','test','account',keys,1,b'2c')

    first = eoslib.N('a')
    keys = struct.pack('QQQQ',first,0,0,0)
    r = eoslib.load('test','test','account',keys,1,0)
    results = struct.unpack('QQQQ',keys)
    print('load keys',eoslib.n2s(results[0]),eoslib.n2s(results[2]))
    print('r',r)

    second = eoslib.N('d')
    keys = struct.pack('QQQQ',0,0,second,0)
    r = eoslib.load('test','test','account',keys,1,1)
    results = struct.unpack('QQQQ',keys)
    print('load keys',eoslib.n2s(results[0]),eoslib.n2s(results[2]))
    print('r',r)

    first = eoslib.N('a')
    second = eoslib.N('c')
    keys = struct.pack('QQQQ',first,0,second,0)
    r = eoslib.front('test','test','account',keys,1,1)
    print('1 front',r)

#    keys = struct.pack('QQQQ',0,0,0,0)
    keys = bytes(32)
    r = eoslib.front('test','test','account',keys,1,1)
    results = struct.unpack('QQQQ',keys)
    print('2 keys',eoslib.n2s(results[0]),eoslib.n2s(results[2]))
    print('2 front',r)

    keys = bytes(32)
    r = eoslib.back('test','test','account',keys,1,1)
    results = struct.unpack('QQQQ',keys)
    print('2 keys',eoslib.n2s(results[0]),eoslib.n2s(results[2]))
    print('back',r)

def init():
#    test_x64x64x64()
#    test_x128x128()
    keys = struct.pack('Q',N('a'))
    values = struct.pack('QQQ',123,456,789)
    r = eoslib.store('test','test','transfer',keys,0,values)
    print(r)
    keys = struct.pack('Q',N('b'))
    values = struct.pack('QQQ',123,456,789)
    r = eoslib.store('test','test','transfer',keys,0,values)
    print(r)
    r = eoslib.load('test','test','transfer',keys,0,0)
    print(r)
    print(struct.unpack('QQQ',r))

def apply(name,type):
#    print('hello from python apply',name,type)
#    print(eoslib.n2s(name),eoslib.n2s(type))
    if type == eoslib.N(b'transfer'):
        pass

if __name__ == '__main__':
    init()
    apply(eoslib.N(b'python'),eoslib.N(b'transfer'))



