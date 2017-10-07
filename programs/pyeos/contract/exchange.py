if __name__ == '__main__':
    import eoslib_dummy as eoslib
    from eoslib_dummy import N,readMessage,writeMessage
else:
    import eoslib
    from eoslib import N,readMessage
import struct
import time

exchange = N(b'exchange')
currency = N(b'currency')
table = N(b'account')

MAX_INT64 = 0xffffffffffffffff
class uint128(int):
    def from_bytes(bs):
        return uint128(int.from_bytes(bs,'little'))
    def __call__(self):
#        return struct.pack('QQ',self & MAX_INT64,self >> 64)
        return self.to_bytes(16,'little')

'''
      AccountName name    = 0;
      uint64_t    number  = 0;
'''
class OrderID(object):
    def __init__(self,bs=None):
        if bs:
            result = struct.unpack('QQ',bs)
            self.name = result[0]
            self.id = result[1]
        else:
            self.name = 0
            self.id = 0
        self.raw_data = None
    def __call__(self):
        self.raw_data = struct.pack('QQ',self.name,self.id)
        return self.raw_data
'''
         "buyer" : "OrderID",
         "price" : "UInt128",
         "quantity" : "UInt64",
         "expiration" : "Time"
'''
class Bid(object):
    def __init__(self):
        self.buyer = OrderID()
        self.price = uint128(0)
        self.quantity = 0
        self.expiration = 0
    def store(self):
        keys = struct.pack('16s16s',self.buyer(),self.price())
        values = struct.pack('QI',self.quantity,self.expiration)
        eoslib.store(exchange,exchange,table,keys,1,values)
    def load_by_order_id(id):
        keys = struct.pack('16s16s',id(),bytes(16))
        r = eoslib.load(exchange,exchange,table,keys,1,0)
        if r:
            bid = Bid()
            bid.buyer = OrderID(keys[:16])
            bid.price = uint128.from_bytes(kes[16:])
            result = struct.unpack('QI',r)
            bid.quantity = result[0]
            bid.expiration = result[1]
            return bid
        else:
            return None
    def load_by_price(price):
        eoslib.load(exchange,exchange,table,price(),1,1)
    def front_by_id():
        keys = bytes(32)
        values = bytes(8+4)
        if eoslib.front(exchange,exchange,table,keys,1,0,values):
            bid = Bid()
            bid.buyer = OrderID(keys[:16])
            bid.price = uint128.from_bytes(kes[16:])
            result = struct.unpack('QI',values)
            bid.quantity = result[0]
            bid.expiration = result[1]
            return bid
        return None
    def back_by_id():
        keys = bytes(32)
        values = bytes(8+4)
        if eoslib.back(exchange,exchange,table,keys,1,0,values):
            bid = Bid()
            bid.buyer = OrderID(keys[:16])
            bid.price = uint128.from_bytes(kes[16:])
            result = struct.unpack('QI',values)
            bid.quantity = result[0]
            bid.expiration = result[1]
            return bid
        return None

'''
         "seller" : "OrderID",
         "price" : "UInt128",
         "quantity" : "UInt64",
         "expiration" : "Time"
'''
class Ask(object):
    def __init__(self):
        self.seller = OrderID(0)
        self.price = uint128(0)
        self.quantity = 0
        self.expiration = 0
    def __call__(self):
        return struct.pack('16s16sQI',self.seller(),self.price(),self.quantity,self.expiration)
    
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
    def __init__(self,owner):
        self.owner = owner
#        self.load()
    def __call__(self):
        return struct.pack('QQQI',self.owner,self.eos_balance,self.currency_balance,self.open_orders)
    def store(self):
        keys = struct.pack("Q",self.owner)
        values = struct.pack('QQI',self.eos_balance,self.currency_balance,self.open_orders)
        eoslib.store(exchange,exchange,table,keys,0,values)
    def load(owner):
        keys = struct.pack("Q",owner)
        result = eoslib.load(exchange,exchange,table,keys,0,0)
        if result:
            self = Account(owner)
            result = struct.unpack('QQI',result)
            self.eos_balance = result[1]
            self.currency_balance = result[2]
            self.open_orders = result[3]
            return self
        return None

'''
      "name" : "BuyOrder",
      "base" : "Bid",
      "fields" : {
         "fill_or_kill" : "UInt8"
       }
'''
class BuyOrder(Bid):
    def __init__(self):
        self.msg = readMessage()
        self.buyer = OrderID(self.msg[:16])
        self.price = uint128.from_bytes(self.msg[16:32])
        self.quantity = int.from_bytes(self.msg[32:40],'little')
        self.expiration = int.from_bytes(self.msg[40:],'little')
        self.fill_or_kill = self.msg[-1]
    def __call__(self):
        return struct.pack('16s16sQIB',self.buyer(),self.price(),self.quantity,self.expiration,self.fill_or_kill)

'''
      "name" : "SellOrder",
      "base" : "Ask",
      "fields" : {
         "fill_or_kill" : "UInt8"
       }
'''
class SellOrder(Ask):
    def __init__(self):
        self.msg = readMessage()
        self.seller = OrderID(self.msg[:16])
        self.price = uint128.from_bytes(self.msg[16:32])
        self.quantity = int.from_bytes(self.msg[32:40],'little')
        self.expiration = int.from_bytes(self.msg[40:],'little')
        self.fill_or_kill = self.msg[-1]
    def __call__(self):
        return struct.pack('16s16sQIB',self.seller(),self.price(),self.quantity,self.expiration,self.fill_or_kill)

def apply_exchange_buy():
    order = BuyOrder()
    bid = order
    print(eoslib.n2s(bid.buyer.name))
    eoslib.requireAuth( bid.buyer.name ); 
    assert bid.quantity > 0, "invalid quantity" ;
    assert bid.expiration > eoslib.now(), "order expired" ;
    print( eoslib.n2s(bid.buyer.name), " created bid for ", order.quantity, " currency at price: ", order.price, "\n" );
    buyer_account = Account.load( bid.buyer.name );
    print('buyer_account:',buyer_account)
    
def apply_exchange_sell():
    order = SellOrder()

def apply_exchange_cancel_buy():
    msg = readMessage()
    id = OrderID(msg)

def apply_exchange_cancel_sell():
    msg = readMessage()
    id = OrderID()

def apply_currency_transfer():
    pass

def apply_eos_transfer():
    pass

def init():
    print(eoslib.now())

def apply(code,action):
    if code == exchange:
        if action == N(b'buy'):
            apply_exchange_buy();
        elif action == N(b'sell'):
            apply_exchange_sell();
        elif action == N(b'cancelbuy'):
            apply_exchange_cancel_buy();
        elif action == N(b'cancelsell'):
            apply_exchange_cancel_sell();
        else:
            assert False, "unknown action";
    elif code == currency:
        if action == N(b'transfer'):
            apply_currency_transfer();
    elif code == N(b'eos'):
        if action == N(b'transfer'): 
            apply_eos_transfer();
if __name__ == '__main__':
    init()
    order = BuyOrder(False)
    order.buyer = OrderID()
    order.buyer.name = eoslib.N(b'buyer')
    order.buyer.id = 0
    
    order.price = uint128(11)
    order.quantity = 12
    order.expiration = int(time.time())+100
    writeMessage(order())
    apply(exchange,N(b'buy'))
    
    order = SellOrder(False)
    order.seller = OrderID()
    order.seller.name = eoslib.N(b'seller')
    order.seller.id = 0
    
    order.price = uint128(11)
    order.quantity = 12
    order.expiration = int(time.time())+100
    writeMessage(order())
    
    apply(exchange,N(b'sell'))

    id = OrderID()
    id.name = eoslib.N(b'buyer')
    id.id = 0
    writeMessage(id())

    apply(exchange,N(b'cancelbuy'))
    apply(exchange,N(b'cancelsell'))
    apply(currency,N(b'transfer'))
    apply(N(b'eos'),N(b'transfer'))
    print('done!')

