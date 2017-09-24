if __name__ == '__main__':
    import eoslib_dummy as eoslib
    from eoslib_dummy import N,readMessage
else:
    import eoslib
    from eoslib import N,readMessage
import struct

exchange = N(b'exchange')
currency = N(b'currency')
'''
        "name" : "AccountName",
        "id"   : "UInt64"
'''
class OrderID(object):
    def __init__(self,bs):
        result = struct.unpack('QQ',bs)
        self.name = result[0]
        self.id = result[1]

'''
         "buyer" : "OrderID",
         "price" : "UInt128",
         "quantity" : "UInt64",
         "expiration" : "Time"
'''
class Bid(object):
    def __init__(self):
        self.msg = readMessage()
        self.buyer = OrderID(self.msg[:16])
        self.price = int.from_bytes(self.msg[16:32],'little')
        self.quantity = int.from_bytes(self.msg[32:40],'little')
        self.expiration = int.from_bytes(self.msg[40:],'little')
        
'''
         "seller" : "OrderID",
         "price" : "UInt128",
         "quantity" : "UInt64",
         "expiration" : "Time"
'''
class Ask(object):
    def __init__(self):
        self.msg = readMessage()
        self.seller = OrderID(self.msg[:16])
        self.price = int.from_bytes(self.msg[16:32],'little')
        self.quantity = int.from_bytes(self.msg[32:40],'little')
        self.expiration = int.from_bytes(self.msg[40:],'little')
'''
      "name" : "Account",
      "fields" : {
         "owner" : "AccountName",
         "eos_balance" : "UInt64",
         "currency_balance" : "UInt64",
         "open_orders" : "UInt32"
       }
'''
class Account(object):
    def __init__(self):
        self.msg = readMessage()
        result = struct.unpack('QQQI',self.msg)
        self.owner = result[0]
        self.eos_balance = result[1]
        self.currency_balance = result[2]
        self.open_orders = result[3]

'''
      "name" : "BuyOrder",
      "base" : "Bid",
      "fields" : {
         "fill_or_kill" : "UInt8"
       }
'''
class BuyOrder(Bid):
    def __init__(self):
        super(BuyOrder,self).__init__()
        self.fill_or_kill = self.msg[-1]

'''
      "name" : "SellOrder",
      "base" : "Ask",
      "fields" : {
         "fill_or_kill" : "UInt8"
       }
'''
class SellOrder(Ask):
    def __init__(self):
        super(SellOrder,self).__init__()
        self.fill_or_kill = self.msg[-1]

def apply_exchange_buy(msg):
    pass

def apply_exchange_sell(msg):
    pass

def apply_exchange_cancel_buy(msg):
    pass

def apply_exchange_cancel_sell(msg):
    pass

def apply_currency_transfer(msg):
    pass

def apply_eos_transfer(msg):
    pass

def init():
    pass

def apply(code,action):
    msg = eoslib.readMessage()
    if code == exchange:
        if action == N(b'buy'):
            apply_exchange_buy(msg);
        elif action == N(b'sell'):
            apply_exchange_sell(msg);
        elif action == N(b'cancelbuy'):
            apply_exchange_cancel_buy(msg);
        elif action == N(b'cancelsell'):
            apply_exchange_cancel_sell(msg);
        else:
            assert False, "unknown action";
    elif code == currency:
        if action == N(b'transfer'):
            apply_currency_transfer(msg);
    elif code == N(b'eos'):
        if action == N(b'transfer'): 
            apply_eos_transfer(msg);
if __name__ == '__main__':
    init()
    apply(exchange,N(b'buy'))
    apply(exchange,N(b'sell'))
    apply(exchange,N(b'cancelbuy'))
    apply(exchange,N(b'cancelsell'))
    apply(currency,N(b'transfer'))
    apply(N(b'eos'),N(b'transfer'))
    print('done!')

