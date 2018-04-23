import ustruct as struct
from eoslib import read_action, unpack, n2s

class transfer(object):
    def __init__(self):
        self.unpack()

    def unpack(self):
        msg = read_action()
        self._from, self.to, self.amount = struct.unpack('QQQ', msg)

        self.precision = msg[24]
        self.symbol = msg[25:32].decode('utf8')
        self.memo = unpack(msg[32:])

    def p(self):
        print(n2s(self._from), n2s(self.to), self.amount, self.precision, self.symbol, self.memo)

class issue(object):
    def __init__(self):
        self.unpack()

    def unpack(self):
        msg = read_action()
        self.to, self.amount = struct.unpack('QQ', msg)
        self.precision = msg[16]
        self.symbol = msg[17:24].decode('utf8')
        self.memo = unpack(msg[24:])

    def p(self):
        print(n2s(self.to), self.amount, self.symbol)

class create():
    def __init__(self):
        self.unpack()

    def unpack(self):
        msg = read_action()
        self.issuer, self.amount = struct.unpack('QQ', msg)
        self.precision = msg[16]
        self.symbol = msg[17:24].decode('utf8')

        self.can_freeze = msg[24]
        self.can_recall = msg[25]
        self.can_whitelist = msg[26]

    def p(self):
        print(n2s(self.issuer), self.amount, self.symbol, self.precision, self.can_freeze, self.can_recall, self.can_whitelist)

