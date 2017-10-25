#dummy source for test in standalone python
import struct
import time

def s2n(name):
    for i in range(len(name) - 1, 8):
        name += b'\x00'
    return struct.unpack('Q', name[:8])[0]

def N(name):
    return s2n(name)
def now():
    return int(time.time())
def n2s(n):
    s = struct.pack('Q', n & 0xffffffffffffffff)
    if s.endswith(b'\x00'):
        end = s.find(b'\x00')
        return s[:end]
    return s

def requireAuth(name):
    pass

msg = None
def writeMessage(msg_):
    global msg
    msg = msg_
    
def readMessage():
    return msg
#    return struct.pack('QQQ',N(b'python'),N(b'inita'),10000)

def requireScope(account):
    pass
def requireNotice(account):
    pass
def store(scope, code, table, keys, key_type, values):
    pass
def store_u64(scope, code, table, key, value):
    pass
def update(scope, code, table, keys, key_type, values):
    pass
def remove(scope, code, table, keys, key_type, value):
    pass
def load(scope, code, table, keys, key_type, scope_index, values):
    return values
def load_u64(scope, code, table, key):
    return 0
def front(scope, code, table, keys, key_type, scope_index, values):
    return values
def back(scope, code, table, keys, key_type, scope_index, values):
    return values
