import eoslib
from eoslib import N,readMessage,requireAuth,now
import struct
import time

exchange = N(b'exchange')
currency = N(b'currency')
table_account = N(b'account')
table_asks = N(b'asks')
table_bids = N(b'bids')

MAX_INT64 = 0xffffffffffffffff

Name = eoslib.n2s

class uint64(int):
    def from_bytes(bs):
        return uint64(int.from_bytes(bs,'little'))
    def __call__(self):
#        return struct.pack('QQ',self & MAX_INT64,self >> 64)
        return self.to_bytes(8,'little')

class uint128(int):
    def from_bytes(bs):
        return uint128(int.from_bytes(bs,'little'))
    def __call__(self):
#        return struct.pack('QQ',self & MAX_INT64,self >> 64)
        return self.to_bytes(16,'little')

class Object(object):
    def __str__(self):
        return str(self.__dict__)
    def __repr__(self):
        return str(self.__dict__)

class Transfer(Object):
    def __init__(self):
        self.msg = readMessage()
        self.from_ = int.from_bytes(self.msg[:8],'little')
        self.to_ = int.from_bytes(self.msg[8:16],'little')
        self.amount = int.from_bytes(self.msg[16:24],'little')
        self.memo = str(self.msg[24:],'utf8')

'''
      "name" : "Account",
      "fields" : {
         "owner" : "AccountName",
         "eos_balance" : "UInt64",
         "currency_balance" : "UInt64",
         "open_orders" : "UInt32"
       }
'''
class Account(Object):
    def __init__(self,owner):
        self.owner = owner
        self.load()
    def __call__(self):
        return struct.pack('QQQI',self.owner,self.eos_balance,self.currency_balance,self.open_orders)
    def save(self):
        keys = struct.pack("Q",self.owner)
        values = struct.pack('QQI',self.eos_balance,self.currency_balance,self.open_orders)
        eoslib.store(exchange,exchange,table_account,keys,0,values)
    def load(self):
        keys = struct.pack("Q",self.owner)
        result = eoslib.load(exchange,exchange,table_account,keys,0,0)
        if result:
            result = result[:20]
            result = struct.unpack('QQI',result)
            self.eos_balance = result[0]
            self.currency_balance = result[1]
            self.open_orders = result[2]
        else:
            self.eos_balance = 0
            self.currency_balance = 0
            self.open_orders = 0


'''
      AccountName name    = 0;
      uint64_t    number  = 0;
'''
class OrderID(Object):
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
class Bid(Object):
    def __init__(self):
        self.buyer = OrderID()
        self.price = uint128(0)
        self.quantity = 0
        self.expiration = 0
    def store(self):
        keys = struct.pack('16s16s',self.buyer(),self.price())
        values = struct.pack('QI',self.quantity,self.expiration)
        eoslib.store(exchange,exchange,table_bids,keys,1,values)
    def remove(self):
        keys = struct.pack('16s16s',self.buyer(),self.price())
        values = struct.pack('QI',self.quantity,self.expiration)
        eoslib.remove(exchange,exchange,table_bids,keys,1,values)

    def load_by_order_id(id):
        keys = struct.pack('16s16s',id(),bytes(16))
        r = eoslib.load(exchange,exchange,table_bids,keys,1,0)
        if r:
            bid = Bid()
            bid.buyer = OrderID(keys[:16])
            bid.price = uint128.from_bytes(keys[16:])
            r = r[:12]
            result = struct.unpack('QI',r)
            bid.quantity = result[0]
            bid.expiration = result[1]
            return bid
        else:
            return None
    def load_by_price(price):
        eoslib.load(exchange,exchange,table_bids,price(),1,1)
    def front_by_id():
        keys = bytes(32)
        values = bytes(8+4)
        if eoslib.front(exchange,exchange,table_bids,keys,1,0,values):
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
        if eoslib.back(exchange,exchange,table_bids,keys,1,0,values):
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
class Ask(Object):
    def __init__(self):
        self.seller = OrderID(0)
        self.price = uint128(0)
        self.quantity = 0
        self.expiration = 0
    def __call__(self):
        return struct.pack('16s16sQI',self.seller(),self.price(),self.quantity,self.expiration)
    def store(self):
        keys = struct.pack('16s16s',self.seller(),self.price())
        values = struct.pack('QI',self.quantity,self.expiration)
        eoslib.store(exchange,exchange,table_bids,keys,1,values)
    def load_by_order_id(id):
        keys = struct.pack('16s16s',id(),bytes(16))
        r = eoslib.load(exchange,exchange,table_bids,keys,1,0)
        if r:
            ask = Ask()
            ask.seller = OrderID(keys[:16])
            ask.price = uint128.from_bytes(keys[16:])
            r = r[:12]
            result = struct.unpack('QI',r)
            ask.quantity = result[0]
            ask.expiration = result[1]
            return ask
        else:
            return None
    def load_by_price(price):
        keys = struct.pack('16s16s',bytes(16),price())
        r = eoslib.load(exchange,exchange,table_bids,keys,1,1)
        if r:
            ask = Ask()
            ask.seller = OrderID(keys[:16])
            ask.price = uint128.from_bytes(keys[16:])
            result = struct.unpack('QI',r)
            ask.quantity = result[0]
            ask.expiration = result[1]
            return ask
        else:
            return None
    def front_by_id():
        keys = bytes(32)
        values = bytes(8+4)
        if eoslib.front(exchange,exchange,table_bids,keys,1,0,values):
            ask = Ask()
            ask.seller = OrderID(keys[:16])
            ask.price = uint128.from_bytes(keys[16:])
            result = struct.unpack('QI',values)
            ask.quantity = result[0]
            ask.expiration = result[1]
            return ask
        return None
    def back_by_id():
        keys = bytes(32)
        values = bytes(8+4)
        if eoslib.back(exchange,exchange,table_bids,keys,1,0,values):
            ask = Ask()
            ask.seller = OrderID(keys[:16])
            ask.price = uint128.from_bytes(keys[16:])
            result = struct.unpack('QI',values)
            ask.quantity = result[0]
            ask.expiration = result[1]
            return ask
        return None
    def front_by_price():
        keys = bytes(32)
        values = bytes(8+4)
        if eoslib.front(exchange,exchange,table_bids,keys,1,1,values):
            ask = Ask()
            ask.seller = OrderID(keys[:16])
            ask.price = uint128.from_bytes(keys[16:])
            result = struct.unpack('QI',values)
            ask.quantity = result[0]
            ask.expiration = result[1]
            return ask
        return None
    def back_by_price():
        keys = bytes(32)
        values = bytes(8+4)
        if eoslib.back(exchange,exchange,table_bids,keys,1,1,values):
            ask = Ask()
            ask.seller = OrderID(keys[:16])
            ask.price = uint128.from_bytes(keys[16:])
            result = struct.unpack('QI',values)
            ask.quantity = result[0]
            ask.expiration = result[1]
            return ask
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

#void match( Bid& bid, Account& buyer, Ask& ask, Account& seller )
def match( bid, buyer, ask, seller ):
    print( "match bid: ", bid, "\nmatch ask: ", ask, "\n");
    
    ask_eos = ask.quantity * ask.price;
    
    fill_amount_eos = min( ask_eos, bid.quantity );
    fill_amount_currency = 0;
    
    if fill_amount_eos == ask_eos: #/// complete fill of ask
        fill_amount_currency = ask.quantity;
    else: #/// complete fill of buy
        fill_amount_currency = fill_amount_eos / ask.price;
    
    
    print( "\n\nmatch bid: ", Name(bid.buyer.name), ":", bid.buyer.number,
           "match ask: ", Name(ask.seller.name), ":", ask.seller.number, "\n\n" );
    
    bid.quantity -= fill_amount_eos;
    seller.eos_balance += fill_amount_eos;
    
    ask.quantity -= fill_amount_currency;
    buyer.currency_balance += fill_amount_currency;

def apply_exchange_buy():
    order = BuyOrder()
    bid = order
    print(eoslib.n2s(bid.buyer.name))
    eoslib.requireAuth( bid.buyer.name )
    assert bid.quantity > 0, "invalid quantity" 
    assert bid.expiration > eoslib.now(), "order expired" 
    print( eoslib.n2s(bid.buyer.name), " created bid for ", order.quantity, " currency at price: ", order.price, "\n" )
    buyer_account = Account( bid.buyer.name )
    print('buyer_account:',buyer_account)
    lowest_ask = Ask.front_by_price()
    if not lowest_ask:
        print( "\n No asks found, saving buyer account and storing bid\n" )
        assert not order.fill_or_kill, "order not completely filled" 
        bid.store()
        buyer_account.open_orders+=1
        buyer_account.save()
        return

    print( "ask: ", lowest_ask, "\n" );
    print( "bid: ", bid, "\n" );
    
    seller_account = Account( lowest_ask.seller.name );
    
    while lowest_ask.price <= bid.price :
       print( "lowest ask <= bid.price\n" );
       match( bid, buyer_account, lowest_ask, seller_account );
    
       if lowest_ask.quantity == 0 and seller_account.open_orders > 0:
          seller_account.open_orders-=1;
          seller_account.save();
          buyer_account.save();
          lowest_ask.remove();
          lowest_ask = Ask.front_by_price()
          if lowest_ask:
             break;
          seller_account = Account( lowest_ask.seller.name );
       else:
          break; # buyer's bid should be filled
    print( "lowest_ask >= bid.price or buyer's bid has been filled\n" );
    
    if bid.quantity and not order.fill_or_kill:
        buyer_account.open_orders+=1;
    buyer_account.save();
    print( "saving buyer's account\n" );
    if bid.quantity:
       print( bid.quantity, " eos left over" );
       assert not order.fill_or_kill, "order not completely filled" ;
       bid.store();
       return;
    print( "bid filled\n" );

def apply_exchange_sell():
    order = SellOrder()
    ask = order;
    requireAuth( ask.seller.name ); 
    
    assert ask.quantity > 0, "invalid quantity";
    assert ask.expiration > now(), "order expired" ;
    
    print( "\n\n", eoslib.n2s(ask.seller.name), " created sell for ", order.quantity, 
           " currency at price: ", order.price, "\n");
    
    existing_ask = Ask.load_by_order_id(ask.seller)
    assert not existing_ask, "order with this id already exists";
    
    seller_account = Account( ask.seller.name );
    seller_account.currency_balance -= ask.quantity;
    
    
    highest_bid = Bid.back_by_price()
    if not highest_bid:
        assert not order.fill_or_kill, "order not completely filled"
        print( "\n No bids found, saving seller account and storing ask\n" );
        ask.store();
        seller_account.open_orders+=1;
        seller_account.save();
        return;
    
    print( "\n bids found, lets see what matches\n" );
    buyer_account = Account( highest_bid.buyer.name );
    
    while highest_bid.price >= ask.price:
        match( highest_bid, buyer_account, ask, seller_account );
        if highest_bid.quantity == 0:
            buyer_account.open_orders-=1;
            seller_account.save();
            buyer_account.save();
            highest_bid.remove();
            highest_bid = Bid.back_by_price()
            if not highest_bid:
               break; 
            buyer_account = Account( highest_bid.buyer.name );
        else:
          break; # buyer's bid should be filled
    
    if ask.quantity and not order.fill_or_kill:
        seller_account.open_orders+=1;
    seller_account.save();
    if ask.quantity:
       assert not order.fill_or_kill, "order not completely filled"
       print( "saving ask\n" );
       ask.store();
       return;
    print( "ask filled\n" );
   

def apply_exchange_cancel_buy():
    msg = readMessage()
    id = OrderID(msg)

def apply_exchange_cancel_sell():
    msg = readMessage()
    order = OrderID()
    requireAuth( order.name )
    
    bid_to_cancel = Bid.load_by_order_id(order)
    assert bid_to_cancel, "bid with this id does not exists"
    buyer_account = Account( order.name )
    buyer_account.eos_balance += bid_to_cancel.quantity
    if buyer_account.open_orders > 0:
        buyer_account.open_orders-=1
    bid_to_cancel.remove()
    buyer_account.save()
    print( "bid removed\n" )


def apply_currency_transfer():
    print('apply_currency_transfer')
    transfer = Transfer()
    if transfer.to_ == exchange:
        account = Account(transfer.from_)
        account.currency_balance += transfer.amount
        account.save()
    elif transfer.from_ == exchange and account.currency_balance >= transfer.amount:
        account = Account(transfer.to_)
        account.currency_balance -= transfer.amount
        account.save()
    else:
        assert False, "notified on transfer that is not relevant to this exchange" 
        
def apply_eos_transfer():
    print('apply_eos_transfer')
    transfer = Transfer()
    if transfer.to_ == exchange:
        account = Account(transfer.from_)
        account.eos_balance += transfer.amount
        account.save()
    elif transfer.from_ == exchange and account.eos_balance >= transfer.amount:
        eoslib.requireAuth(transfer.to_)
        account = Account(transfer.to_)
        account.eos_balance -= transfer.amount
        account.save()
    else:
        assert False, "notified on transfer that is not relevant to this exchange"

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


