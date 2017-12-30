import eoslib
try:
    import struct
except Exception as e:
#load struct module in micropython
    import ustruct as struct 

code = eoslib.N(b'currency')
table = eoslib.N(b'account')

class Account(object):
    key = eoslib.N(b'account')
    key = int.to_bytes(key, 8, 'little')
    def __init__(self, scope, balance=0): 
        self.scope = scope
        if balance == 0:
            self.load() 
        else:
            self.balance = balance

    def isEmpty(self):
        return self.balance == 0

    def store(self):
        value = int.to_bytes(self.balance, 8, 'little')
        eoslib.store(self.scope, table, Account.key, 0, value)
    
    def load(self):
        value = bytes(8)
        eoslib.load(self.scope, code, table, Account.key, 0, 0, value)
        self.balance = int.from_bytes(value, 'little')

def init():
    print('hello from currency.init')
    a = Account(code)
    # avoid overwrite balance already exists.
    if a.balance == 0:
        a.balance = 100000
        a.store()

def apply(name, type):
#    print('hello from python apply',name,type)
    print(eoslib.n2s(name),eoslib.n2s(type))
    if type == eoslib.N(b'transfer'):
        msg = eoslib.read_message()
        result = struct.unpack('QQQ', msg)
#         print(result)
        from_ = result[0]
        to_ = result[1]
        amount = result[2]

        eoslib.require_auth(from_);
        eoslib.require_notice(from_);
        eoslib.require_notice(to_)

        from_ = Account(from_)
        to_ = Account(to_)
        if from_.balance >= amount:
            from_.balance -= amount
            to_.balance += amount
            from_.store()
            to_.store()

