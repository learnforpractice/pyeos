from eoslib import read_action, N, n2s, unpack

try:
    import ustruct as struct 
except Exception as e:
    import struct

code = N(b'currency')
table = N(b'account')

class transfer(object):
    def __init__(self):
        self.unpack()

    def unpack(self):
        msg = read_action()
        self._from, self.to, self.amount = struct.unpack('QQQ', msg)

        self.precision = msg[24]
        self.symbol = msg[25:32]
        self.memo = msg[32:]

    def p(self):
        print(n2s(self._from), n2s(self.to), self.amount, self.precision, self.symbol, unpack(self.memo))

class issue(object):
    def __init__(self):
        self.unpack()

    def unpack(self):
        msg = read_action()
        self.to, self.amount = struct.unpack('QQ', msg)
        self.precision = msg[16]
        self.symbol = msg[17:24]
        self.memo = msg[24:]

    def p(self):
        print(n2s(self.to), self.amount, self.symbol)

class create():
    def __init__(self):
        self.unpack()

    def unpack(self):
        msg = read_action()
        self.issuer, self.amount = struct.unpack('QQ', msg)
        self.precision = msg[16]
        self.symbol = msg[17:24]

        self.can_freeze = msg[24]
        self.can_recall = msg[25]
        self.can_whitelist = msg[26]

    def p(self):
        print(n2s(self.issuer), self.amount, self.symbol, self.precision, self.can_freeze, self.can_recall, self.can_whitelist)

#{"to":"currency","quantity":"1000.0000 CUR","memo":""}
def issue_():
    iss = issue()
#    iss.p()

#{"from":"currency","to":"eosio","quantity":"20.0000 CUR","memo":"my first transfer"}
def transfer_():
    t = transfer()
#    t.p()

def create_():
    c = create()
#    c.p()

def apply(name, type):
    return
#    print('++++++apply:', n2s(name))
    if type == N('transfer'):
        transfer_()
    elif type == N('issue'):
        issue_()
    elif type == N('create'):
        create_()

