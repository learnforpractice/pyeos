import ustruct as struct
import eoslib

class transfer(object):
    def __init__(self):
        self._from, self.to, self.amount = 0, 0, 0

        self.precision = 0
        self.symbol = None
        self.memo = None

    def unpack(self, msg):
        self._from, self.to, self.amount = struct.unpack('QQQ', msg)

        self.precision = msg[24]
        self.symbol = msg[25:32].decode('utf8')
        self.memo = eoslib.unpack(msg[32:])

    def pack(self):
        result = struct.pack('QQQ', self._from, self.to, self.amount)
        result += chr(self.precision)
        symbol = bytearray(7)
        for i in range(len(self.symbol)):
            symbol[i] = ord(self.symbol[i])
        result += bytes(symbol)
        result += eoslib.pack(self.memo)
        return result

    def p(self):
        print(eoslib.n2s(self._from), eoslib.n2s(self.to), self.amount, self.precision, self.symbol, self.memo)

class issue(object):
    def __init__(self):
        self.unpack()

    def unpack(self):
        msg = eoslib.read_action()
        self.to, self.amount = struct.unpack('QQ', msg)
        self.precision = msg[16]
        self.symbol = msg[17:24].decode('utf8')
        self.memo = eoslib.unpack(msg[24:])

    def p(self):
        print(eoslib.n2s(self.to), self.amount, self.symbol)

class create():
    def __init__(self):
        self.unpack()

    def unpack(self):
        msg = eoslib.read_action()
        self.issuer, self.amount = struct.unpack('QQ', msg)
        self.precision = msg[16]
        self.symbol = msg[17:24].decode('utf8')

        self.can_freeze = msg[24]
        self.can_recall = msg[25]
        self.can_whitelist = msg[26]

    def p(self):
        print(eoslib.n2s(self.issuer), self.amount, self.symbol, self.precision, self.can_freeze, self.can_recall, self.can_whitelist)

