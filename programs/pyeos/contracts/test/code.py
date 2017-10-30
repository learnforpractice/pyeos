import eoslib
import struct
from eoslib import N
def init():
    print('hello from init')
#    eoslib.call_wasm_function(N('test2'), N(b'hello'), [1,2])

test = N(b'test')

def test_call_wasm_function():
    eoslib.call_wasm_function(N('test2'), N(b'hello'), [1,2])

def test_db1():
    msg = eoslib.readMessage()
    print(len(msg))
    result = int.from_bytes(msg[:8], 'little')
    size = msg[8]
    for i in range(size):
        result = int.from_bytes(msg[9 + i * 8:9 + i * 8 + 8], 'little')
        print(result)
    keys = msg[:8]
    values = msg[8:]
    eoslib.store(test, test, test, keys, 0, values)
    keys = msg[:8]
    values = msg[8:]
    eoslib.load(test, test, test, keys, 0, 0, values)
    print(values)

def test_db():
    for i in range(4,11):
        keys = int.to_bytes(i+1,8,'little')
        values = int.to_bytes(i+2,8,'little')
        eoslib.store(test, test, test, keys, 0, values)

    print('----------upper bound------------')
    keys = int.to_bytes(8,8,'little')
    for i in range(4):
        values = bytes(8)
        ret = eoslib.upper_bound(test, test, test, keys, 0,0,values)
        print(ret)
        print('keys:',int.from_bytes(keys,'little'))
        print('values',int.from_bytes(values,'little'))

    print('----------upper bound------------')
    keys = int.to_bytes(8,8,'little')
    values = bytes(8)
    ret = eoslib.upper_bound(test, test, test, keys, 0,0,values)
    print(ret)
    print('keys:',int.from_bytes(keys,'little'))
    print('values',int.from_bytes(values,'little'))

    values = bytes(8)
    ret = eoslib.upper_bound(test, test, test, keys, 0,0,values)
    print(ret)
    print('keys:',int.from_bytes(keys,'little'))
    print('values',int.from_bytes(values,'little'))

    values = bytes(8)
    ret = eoslib.upper_bound(test, test, test, keys, 0,0,values)
    print(ret)
    print('keys:',int.from_bytes(keys,'little'))
    print('values',int.from_bytes(values,'little'))

    print('----------lower bound------------')
    keys = int.to_bytes(0,8,'little')
    for i in range(4):
        values = bytes(8)
        ret = eoslib.lower_bound(test, test, test, keys, 0,0,values)
        print(ret)
        print('keys:',int.from_bytes(keys,'little'))
        print('values',int.from_bytes(values,'little'))

    print('----------lower bound------------')
    keys = int.to_bytes(1,8,'little')
    values = bytes(8)
    ret = eoslib.lower_bound(test, test, test, keys, 0,0,values)
    print(ret)
    print('keys:',int.from_bytes(keys,'little'))
    print('values',int.from_bytes(values,'little'))

    keys = int.to_bytes(1,8,'little')
    values = bytes(8)
    ret = eoslib.lower_bound(test, test, test, keys, 0,0,values)
    print(ret)
    print('keys:',int.from_bytes(keys,'little'))
    print('values',int.from_bytes(values,'little'))

    keys = int.to_bytes(1,8,'little')
    values = bytes(8)
    ret = eoslib.lower_bound(test, test, test, keys, 0,0,values)
    print(ret)
    print('keys:',int.from_bytes(keys,'little'))
    print('values',int.from_bytes(values,'little'))

    print('----------------end----------------------')

def test_transaction():
    print("------------------test_transaction---------------")
    tshandle = eoslib.transactionCreate()
    eoslib.transactionRequireScope(tshandle,b'test',0)
    eoslib.transactionRequireScope(tshandle,b'inita',0)

#'{"from":"test","to":"inita","amount":50}'
    data = struct.pack("QQQ",N(b'test'),N(b'inita'),50)
    msghandle = eoslib.messageCreate(b'currency', b'transfer', data)
    eoslib.messageRequirePermission(msghandle,b'test',b'active')

    eoslib.transactionAddMessage(tshandle,msghandle)
    eoslib.transactionSend(tshandle)

def test_message():
#'{"from":"currency","to":"test","amount":50}'
    data = struct.pack("QQQ",N(b'test'),N(b'inita'),50)
    msghandle = eoslib.messageCreate(b'currency', b'transfer', data)
    eoslib.messageRequirePermission(msghandle,b'test',b'active')
    eoslib.messageSend(msghandle)

def apply(code, action):
    print(code,action)
    if code == test:
        eoslib.requireAuth(test)
        if action == N(b'transfer'):
            msg = eoslib.readMessage()
            result = struct.unpack('QQQ', msg)
            print(result)
            from_ = result[0]
            to_ = result[1]
            amount = result[2]
        elif action == N(b'test'):
            test_db2()
        elif action == N(b'testdb'):
            test_db()
        elif action == N(b'callwasm'):
        		test_call_wasm_function()
        elif action == N(b'testmsg'):
            print('testmsg')
            test_message()
        elif action == N(b'testts'):
            test_transaction()
