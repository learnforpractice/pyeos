import eoslib
import struct
import pickle
from eoslib import N
def init():
    print('hello from test.init')
#    eoslib.call_wasm_function(N('test2'), N(b'hello'), [1,2])

test = N(b'test')

def test_call_wasm_function():
    eoslib.call_wasm_function(N('test2'), N(b'hello'), [1,2])

def test_rw_db():
    keys = struct.pack('Q', N('currency'))
    values = bytes(16)
    eos = N('eos')
    ret = eoslib.load(eos, eos, N('test'), keys, 0, 0, values)
    print('+++++++eoslib.load return:',ret)
    print(values)
    results = struct.unpack('QQ', values)
    print(results)

    print('--------------------------------')
    keys = struct.pack('Q', N('currency'))
    values = struct.pack('QQ', 77, 88)
    ret = eoslib.store(test, eos, N('test'), keys, 0, values)
    print('++++++++eoslib.store return:',ret)

    values = bytes(16)
    ret = eoslib.load(test, eos, N('test'), keys, 0, 0, values)
    print('+++++++eoslib.load return:',ret)
    print(values)
    results = struct.unpack('QQ', values)
    print(results)

    print('--------------------------------')

    keys = struct.pack('Q', N('currency'))
    values = struct.pack('QQ', 44, 33)
    ret = eoslib.store(test, test, N('test'), keys, 0, values)
    print('++++++++eoslib.store return:',ret)

    values = bytes(16)
    ret = eoslib.load(test, test, N('test'), keys, 0, 0, values)
    print('+++++++eoslib.load return:',ret)
    print(values)
    results = struct.unpack('QQ', values)
    print(results)


def test_db1():
    msg = eoslib.readMessage()
    print(len(msg))
    test = N(b'eos')
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
    
def test_db2():
    msg = eoslib.readMessage()
    print(len(msg))
    test = N(b'eos')
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
    for i in range(4, 11):
        keys = int.to_bytes(i + 1, 8, 'little')
        values = int.to_bytes(i + 2, 8, 'little')
        eoslib.store(test, test, test, keys, 0, values)

    print('----------upper bound------------')
    keys = int.to_bytes(8, 8, 'little')
    for i in range(4):
        values = bytes(8)
        ret = eoslib.upper_bound(test, test, test, keys, 0, 0, values)
        print(ret)
        print('keys:', int.from_bytes(keys, 'little'))
        print('values', int.from_bytes(values, 'little'))

    print('----------upper bound------------')
    keys = int.to_bytes(8, 8, 'little')
    values = bytes(8)
    ret = eoslib.upper_bound(test, test, test, keys, 0, 0, values)
    print(ret)
    print('keys:', int.from_bytes(keys, 'little'))
    print('values', int.from_bytes(values, 'little'))

    values = bytes(8)
    ret = eoslib.upper_bound(test, test, test, keys, 0, 0, values)
    print(ret)
    print('keys:', int.from_bytes(keys, 'little'))
    print('values', int.from_bytes(values, 'little'))

    values = bytes(8)
    ret = eoslib.upper_bound(test, test, test, keys, 0, 0, values)
    print(ret)
    print('keys:', int.from_bytes(keys, 'little'))
    print('values', int.from_bytes(values, 'little'))

    print('----------lower bound------------')
    keys = int.to_bytes(0, 8, 'little')
    for i in range(4):
        values = bytes(8)
        ret = eoslib.lower_bound(test, test, test, keys, 0, 0, values)
        print(ret)
        print('keys:', int.from_bytes(keys, 'little'))
        print('values', int.from_bytes(values, 'little'))

    print('----------lower bound------------')
    keys = int.to_bytes(1, 8, 'little')
    values = bytes(8)
    ret = eoslib.lower_bound(test, test, test, keys, 0, 0, values)
    print(ret)
    print('keys:', int.from_bytes(keys, 'little'))
    print('values', int.from_bytes(values, 'little'))

    keys = int.to_bytes(1, 8, 'little')
    values = bytes(8)
    ret = eoslib.lower_bound(test, test, test, keys, 0, 0, values)
    print(ret)
    print('keys:', int.from_bytes(keys, 'little'))
    print('values', int.from_bytes(values, 'little'))

    keys = int.to_bytes(1, 8, 'little')
    values = bytes(8)
    ret = eoslib.lower_bound(test, test, test, keys, 0, 0, values)
    print(ret)
    print('keys:', int.from_bytes(keys, 'little'))
    print('values', int.from_bytes(values, 'little'))

    print('----------------end----------------------')

def test_transaction():
    print("------------------test_transaction---------------")
    #transaction will apply in the next block
    tshandle = eoslib.transactionCreate()
    eoslib.transactionRequireScope(tshandle, b'test', 0)
    eoslib.transactionRequireScope(tshandle, b'inita', 0)

# '{"from":"test","to":"inita","amount":50}'
    data = struct.pack("QQQ", N(b'test'), N(b'inita'), 50)
    msghandle = eoslib.messageCreate(b'currency', b'transfer', data)
    eoslib.messageRequirePermission(msghandle, b'test', b'active')
    eoslib.transactionAddMessage(tshandle, msghandle)

#{"from":"test", "to":"inita", "amount":50, "memo":"hello"}
    data = struct.pack("QQQ", N(b'test'), N(b'inita'), 50)
    data += eoslib.pack(b'hello')
    msghandle = eoslib.messageCreate(b'eos', b'transfer', data)
    eoslib.messageRequirePermission(msghandle, b'test', b'active')
    eoslib.transactionAddMessage(tshandle, msghandle)

    eoslib.transactionSend(tshandle)
    print('-------end-------------')

def test_message():
# '{"from":"currency","to":"test","amount":50}'
    data = struct.pack("QQQ", N(b'test'), N(b'inita'), 50)
    msghandle = eoslib.messageCreate(b'currency', b'transfer', data)
    eoslib.messageRequirePermission(msghandle, b'test', b'active')
    eoslib.messageSend(msghandle)

#{"from":"test", "to":"inita", "amount":50, "memo":"hello"}
    data = struct.pack("QQQ", N(b'test'), N(b'inita'), 50)
    data += eoslib.pack(b'hello')
    msghandle = eoslib.messageCreate(b'eos', b'transfer', data)
    eoslib.messageRequirePermission(msghandle, b'test', b'active')
    eoslib.messageSend(msghandle)

def test_memory_limit():
    msg = eoslib.readMessage()
    print(msg,len(msg))
    size = int.from_bytes(msg[:8],'little')
    print('+++++++++++memory size:',size)
    arr = []
    for i in range(int(size/1024)):
        a = bytes(1024)
        if i%100 == 0:
            print(i)
        arr.append(a)

def test_time_out():
    while True:
        pass

def test_exec_code():
    fail = False
    try:
        import json
    except Exception as e:
        print(e)
        fail = True

    try:
        exec("print('hello')")
    except Exception as e:
        print(e)
        fail = True

    try:
        eval('1+1')
    except Exception as e:
        print(e)
        fail = True

    if fail:
        raise Exception("test passed")

def test_import():
    try:
        import traceback
    except Exception as e:
        print(e)
        raise Exception('import traceback failed')

def apply(code, action):
#    print(eoslib.n2s(code),eoslib.n2s(action))
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
            test_rw_db()
        elif action == N(b'testdb'):
            test_db()
        elif action == N(b'callwasm'):
        		test_call_wasm_function()
        elif action == N(b'testmsg'):
            print('testmsg')
            test_message()
        elif action == N(b'testts'):
            test_transaction()
        elif action == N(b'testmem'):
            test_memory_limit()
        elif action == N(b'testtimeout'):
            test_time_out()
        elif action == N(b'testexec'):
            test_exec_code()
        elif action == N(b'testimport'):
            test_import()

