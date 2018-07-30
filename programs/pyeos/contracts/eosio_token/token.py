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
        buffer = bytearray(16)
        struct.pack_into('Q8s', buffer, 0, self.amount, self.symbol)
        return bytes(buffer)

    @classmethod
    def unpack(cls, data):
        a = asset()
        a.amount, a.symbol = struct.unpack('Q8s', data)
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
        result = bytearray(40)
        s1 = self.supply
        s2 = self.max_supply
        struct.pack_into('Q8sQ8sQ', result, 0, s1.amount, s1.symbol, s2.amount, s2.symbol, self.issuer)
        return bytes(result)

    @classmethod
    def unpack(cls, data):
        cs = currency_stats()
        s1 = asset()
        s2 = asset()
        s1.amount, s1.symbol, s2.amount, s2.symbol, cs.issuer = struct.unpack_from('Q8sQ8sQ', data)
        cs.supply = s1
        cs.max_supply = s2
        return cs

class transfer(object):
    def __init__(self):
        self._from, self.to, self.amount = 0, 0, 0

        self.symbol = None
        self.memo = None

    def unpack(self, msg):
        self._from, self.to, self.amount, self.symbol = struct.unpack('QQQ8s', msg)
        self.memo = eoslib.unpack_bytes(msg[32:])

    def pack(self):
        result = struct.pack('QQQ8s', self._from, self.to, self.amount, self.symbol)
        result += eoslib.pack_bytes(self.memo)
        return result

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
        self.issuer, self.amount, self.symbol = struct.unpack('QQ8s', msg)

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
        eoslib.require_auth(code)
        msg = eoslib.read_action()
        issuer, amount, symbol = struct.unpack('QQ8s', msg)
        table_id = N('stat')
        payer = code
        primary_key = scope = to_symbol_name(symbol)
        table_id = N('stat')
        it = db.find_i64(code, scope, table_id, primary_key)
        if it >= 0:
            raise Exception('currency has already been created')

        result = bytearray(40)
        struct.pack_into('Q8sQ8sQ', result, 0, 0, symbol, amount, symbol, issuer)
        result = bytes(result)
        db.store_i64(scope, table_id, payer, primary_key, result)

    elif act == N('issue'):
        msg = eoslib.read_action()
        _to, amount, symbol = struct.unpack('QQ8s', msg[:24])
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
        eoslib.require_auth(cs.issuer)
#        print(cs.supply.amount)
        cs.supply.amount += amount
        assert cs.supply.amount < cs.max_supply.amount
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
        _from, to, amount, symbol = struct.unpack('QQQ8s', msg[:32])
        eoslib.require_auth(_from)
        memo = eoslib.unpack_bytes(msg[32:])
        a1 = Balance(_from, symbol)
        a2 = Balance(to, symbol)
        a1.sub(amount)
        a2.add(amount, _from)

