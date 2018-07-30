import struct

import db
from eoslib import N
import eoslib

code = N('eosio.token')
#code = N('mytoken')

def to_symbol_name(s):
    n = int.from_bytes(s, 'little')
    return n>>8

class asset(object):
    def __init__(self, amount=0, symbol=None):
        self.amount = amount
        self.symbol = symbol

    def pack(self):
        result = struct.pack('Q', self.amount)
        result += self.symbol
        return result

    @classmethod
    def unpack(cls, data):
        a = asset()
        a.amount, = struct.unpack('Q', data[:8])
        a.symbol = data[8:]
        return a

class multi_index(object):
    def __init__(self, code, scope, table_id):
        pass

class currency_stats(object):
    def __init__(self, supply=None, max_supply=None, issuer=0):
        self.supply = supply
        self.max_supply = max_supply
        self.issuer = issuer

    def store(self):
        payer = code
        primary_key = scope = to_symbol_name(self.supply.symbol)
        table_id = N('stat')
        it = db.find_i64(code, scope, table_id, primary_key)
        if it < 0:
            raise Exception('currency already been created')
        db.store_i64(scope, table_id, payer, primary_key, self.pack())

    def get_primary_key(self):
        return to_symbol_name(self.supply.symbol)

    def pack(self):
        result = self.supply.pack()
        result += self.max_supply.pack()
        result += struct.pack('Q', self.issuer)
        return result

    @classmethod
    def unpack(cls, data):
        cs = currency_stats()
        cs.supply = asset.unpack(data[:16])
        cs.max_supply = asset.unpack(data[16:32])
        cs.issuer, = struct.unpack('Q', data[32:40])
        return cs

class transfer(object):
    def __init__(self):
        self._from, self.to, self.amount = 0, 0, 0

        self.symbol = None
        self.memo = None

    def unpack(self, msg):
        self._from, self.to, self.amount = struct.unpack('QQQ', msg)
        self.symbol = msg[24:32].decode('utf8')
        self.memo = eoslib.unpack(msg[32:])

    def pack(self):
        result = struct.pack('QQQ', self._from, self.to, self.amount)
        result += self.symbol
        result += eoslib.pack_bytes(self.memo)
        return result

    def p(self):
        print(eoslib.n2s(self._from), eoslib.n2s(self.to), self.amount, self.precision, self.symbol, self.memo)

'''
    {"name":"to", "type":"account_name"},
    {"name":"quantity", "type":"asset"},
    {"name":"memo", "type":"string"}
'''

class issue(object):
    def __init__(self):
        self.unpack()

    def pack(self):
        pass

    def unpack(self):
        msg = eoslib.read_action()
        self.to, self.amount = struct.unpack('QQ', msg)
        self.symbol = msg[16:24]
        self.memo = eoslib.unpack_bytes(msg[24:])

    def p(self):
        print(eoslib.n2s(self.to), self.amount, self.symbol)

'''
    {"name":"issuer", "type":"account_name"},
    {"name":"maximum_supply", "type":"asset"}
'''
class create():
    def __init__(self):
        self.unpack()

    def pack(self):
        pass
    
    def unpack(self):
        msg = eoslib.read_action()
        self.issuer, self.amount = struct.unpack('QQ', msg)
        self.symbol = msg[16:24]

    def p(self):
        print(eoslib.n2s(self.issuer), self.amount, self.symbol)

'''
{"name":"to", "type":"account_name"},
{"name":"quantity", "type":"asset"},
{"name":"memo", "type":"string"}
'''

class Balance(object):
    def __init__(self, name, symbol):
        self.owner = name
        self.symbol = symbol
        self.amount = 0
        self.symbol_name = to_symbol_name(symbol)
        self.table_id = N('accounts')

        a = asset(self.amount, self.symbol)
        data = a.pack()
        it = db.find_i64(code, self.owner, self.table_id, self.symbol_name)
        if it >= 0:
            data = db.get_i64(it)
            self.a = asset.unpack(data)
        else:
            self.a = asset(0, symbol)

    def add(self, amount, payer):
        self.a.amount += amount
        self.store(payer)

    def sub(self, amount):
        assert(self.a.amount >= amount)
        self.a.amount -= amount
        self.store()

    def store(self, payer=None):
        data = self.a.pack()
        if not payer:
            payer = self.owner
        it = db.find_i64(code, self.owner, self.table_id, self.symbol_name)
        if it < 0:
            db.store_i64(self.owner, self.table_id, payer, self.symbol_name, data)
        else:
            db.update_i64(it, payer, data)

def apply(receiver, account, act):

    if act == N('create'):
        msg = eoslib.read_action()
        print('++++++++++++++:', msg)
        issuer, amount = struct.unpack('QQ', msg[:16])
        symbol = msg[16:24]

        table_id = N('stat')
        payer = code
        primary_key = scope = to_symbol_name(symbol)
        table_id = N('stat')
        it = db.find_i64(code, scope, table_id, primary_key)
        if it >= 0:
            raise Exception('currency has already been created')

        result = struct.pack('Q', 0)
        result += symbol
        result += struct.pack('Q', amount)
        result += symbol
        result += struct.pack('Q', issuer)
        db.store_i64(scope, table_id, payer, primary_key, result)

    elif act == N('issue'):
        msg = eoslib.read_action()
        _to, amount = struct.unpack('QQ', msg[:16])
        symbol = msg[16:24]
        memo = eoslib.unpack_bytes(msg[24:])

        table_id = N('stat')
        payer = code
        symbol_name = primary_key = scope = to_symbol_name(symbol)
        table_id = N('stat')
        it = db.find_i64(code, scope, table_id, primary_key)
        if it < 0:
            raise Exception('currency does not exists')

        data = db.get_i64(it)
        cs = currency_stats.unpack(data)
#        print(cs.supply.amount)
        cs.supply.amount += amount
        db.update_i64(it, payer, cs.pack())

        table_id = N('accounts')

        it = db.find_i64(code, _to, table_id, primary_key)
        if it >= 0:
            data = db.get_i64(it)
            a = asset.unpack(data)
            a.amount += amount
            db.update_i64(it, _to, a.pack())
        else:
            a = asset(amount, symbol)
            db.store_i64(_to, table_id, _to, symbol_name, a.pack())
    elif act == N('transfer'):
        msg = eoslib.read_action()
        _from, to, amount = struct.unpack('QQQ', msg[:24])
        eoslib.require_auth(_from)
        symbol = msg[24:32]
        memo = eoslib.unpack_bytes(msg[32:])
        a1 = Balance(_from, symbol)
        a2 = Balance(to, symbol)
        a1.sub(amount)
        a2.add(amount, _from)

