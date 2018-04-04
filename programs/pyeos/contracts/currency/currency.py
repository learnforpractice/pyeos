from eoslib import read_action, N

try:
    import ustruct as struct 
except Exception as e:
    import struct

code = N(b'currency')
table = N(b'account')

#{"to":"currency","quantity":"1000.0000 CUR","memo":""}
def issue():
    msg = read_action()
    _to, _amount, _symbol = struct.unpack('QQQ', msg)
    memo = msg[24:]
    print(_to, _amount, _symbol)

#{"from":"currency","to":"eosio","quantity":"20.0000 CUR","memo":"my first transfer"}
def transfer():
    msg = read_action()
    _from, _to, _amount, _symbol = struct.unpack('QQQQ', msg)
    memo = msg[32:]
    print(_from, _to, _amount, _symbol)

def create():
    msg = read_action()
    _issuer, _amount, _symbol = struct.unpack('QQQ', msg)
    _can_freeze = msg[25]
    _can_recall = msg[26]
    _can_whitelist = msg[27]
    print(_issuer, _amount, _symbol, _can_freeze, _can_recall, _can_whitelist)

def apply(name, type):
    if type == N('transfer'):
        transfer()
    elif type == N('issue'):
        issue()
    elif type == n('create'):
        create()


