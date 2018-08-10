#porting from http://solidity.readthedocs.io/en/latest/solidity-by-example.html
import struct
from eoslib import *
require = eosio_assert

'''
    function SimpleAuction(
        uint _biddingTime,
        address _beneficiary
    ) public {
        beneficiary = _beneficiary;
        auctionEnd = now + _biddingTime;
    }
'''

_code = N('auction1')


def event(func):
    def func_wrapper(self, *args):
        print('TODO: event')
        return func(self, *args)
    return func_wrapper


class SList(object):
    def __init__(self, code, scope, table_id):
        self.code = code
        self.scope = scope
        self.table_id = table_id
        self.count = db.get_table_item_count(code, scope, table_id)
        print('table count:', self.count)

    def pack(self, obj):
        if type(obj) is int:
            return struct.pack('QB', obj, 0)
        elif type(obj) is bytes:
            return struct.pack('%dsB'%(len(obj),), obj, 1)
        raise Exception('unsupported type', type(obj))
    
    def unpack(self, bs):
        if bs[-1] == 0:
            return struct.unpack('Q', bs[:8])[0]
        elif bs[-1] == 1:
            return bs[:-1]

    def __setitem__(self, index, val):
        if index < 0 or index >= self.count:
            raise IndexError('list index out of range')
        db.update_i64(index, self.code, self.pack(val))

    def __getitem__(self, index):
        if index < 0 or index >= self.count:
            raise IndexError('list index out of range')
        print('index', index)
        itr = db.find_i64(self.code, self.scope, self.table_id, index)
        if itr < 0:
            raise Exception('index not found!')
        print('index', index, 'it', itr)
        value = db.get_i64(itr)
        return self.unpack(value)

    def append(self, value):
        print('+++append:', value)
        db.store_i64(self.scope, self.table_id, self.code, self.count, self.pack(value))
        self.count += 1
        print('+++append return:', self.count)

    def __len__(self):
        return self.count

class SDict(object):
    def __init__(self, code, scope, table_id):
        self.code = code
        self.scope = scope
        self.table_id = table_id
        self.count = db.get_table_item_count(code, scope, table_id)
        print('table count:', self.count)

    def pack(self, obj):
        if type(obj) is int:
            return struct.pack('QB', obj, 0)
        elif type(obj) is bytes:
            return struct.pack('%dsB'%(len(obj),), obj, 1)
        raise Exception('unsupported type', type(obj))

    def unpack(self, bs):
        if bs[-1] == 0:
            return struct.unpack('Q', bs[:8])[0]
        elif bs[-1] == 1:
            return bs[:-1]

    def __setitem__(self, index, val):
        itr = db.find_i64(self.code, self.scope, self.table_id, index)
        if itr > 0:
            db.update_i64(index, self.code, self.pack(val))
        else:
            db.store_i64(self.scope, self.table_id, self.code, index, self.pack(val))

    def __getitem__(self, index):
        itr = db.find_i64(self.code, self.scope, self.table_id, index)
        if itr < 0:
            raise Exception('index not found!')
        print('index', index, 'it', itr)
        value = db.get_i64(itr)
        return self.unpack(value)

    def __len__(self):
        return self.count

class Database(object):
    def __init__(self, code, scope, table_id):
        self.code = code
        self.scope = scope
        self.table_id = table_id

    def store(self, payer=None):
        if not payer:
            payer = self.code
        it = db.find_i64(self.code, self.scope, self.table_id, self.get_primary_key())
        if it >= 0:
            db.update_i64(it, payer, self.pack())
        else:
            db.store_i64(self.scope, self.table_id, payer, self.get_primary_key(), self.pack())

    def load(self):
        it = db.find_i64(self.code, self.scope, self.table_id, self.get_primary_key())
        if it >= 0:
            data = db.get_i64(it)
            return self.unpack(data)
        return False

    def erase(self):
        it = db.find_i64(self.code, self.scope, self.table_id, self.get_primary_key())
        if it >= 0:
            db.remove_i64(it)

    def get_primary_key(self):
        raise Exception('should be implemented by subclass')

    def pack(self):
        raise Exception('should be implemented by subclass')

    def unpack(self, data):
        raise Exception('should be implemented by subclass')

class Singleton(Database):
    def __init__(self, code, scope, table_id):
        super(Singleton, self).__init__(code, scope, table_id)
        self.pk_value = table_id

    def get_primary_key(self):
        return self.pk_value

class SimpleAuction(Singleton):
    def __init__(self):
        super(SimpleAuction, self).__init__(_code, _code, _code)
        self.beneficiary = 0 #address(0)
        self.auctionEnd = 0

        # Current state of the auction.
        self.highestBidder = 0
        self.highestBid = 0

        # Set to true at the end, disallows any change
        self.ended = 0
        self.locked = 0
        try:
            self.load()
        except:
            pass
    def start(self):
        print('+++self.auctionEnd:', self.auctionEnd)
        require(self.auctionEnd <=0, 'auction already started.')
        msg = read_action()
        self.beneficiary, self.auctionEnd = struct.unpack('QQ', msg)
        require(self.auctionEnd > now())
        self.store()

    # End the auction and send the highest bid
    # to the beneficiary.
    def end(self):
        # It is a good guideline to structure functions that interact
        # with other contracts (i.e. they call functions or send Ether)
        # into three phases:
        # 1. checking conditions
        # 2. performing actions (potentially changing conditions)
        # 3. interacting with other contracts
        # If these phases are mixed up, the other contract could call
        # back into the current contract and modify the state or cause
        # effects (ether payout) to be performed multiple times.
        # If functions called internally include interaction with external
        # contracts, they also have to be considered interaction with
        # external contracts.

        # 1. Conditions
        require(now() >= self.auctionEnd, "Auction not yet ended.");
        require(not self.ended, "auctionEnd has already been called.");

        # 2. Effects
        self.ended = 1;

        # 3. Interaction
        transfer(self.code, self.beneficiary, self.highestBid)
        self.store()

    def reset(self):
        require(not self.locked)

        if self.highestBid:
            transfer(self.code, self.highestBidder, self.highestBid)

        self.beneficiary = 0
        self.auctionEnd = 0
        self.highestBidder = 0
        self.highestBid = 0
        self.ended = 0

        self.store()

    def bid(self, sender, value):
        require(self.beneficiary, 'Auction not started yet')
        require(now() <= self.auctionEnd, "Auction already ended.")

        # If the bid is not higher, send the money back.
        require(value > self.highestBid, "There already is a higher bid.")

        if self.highestBid != 0:
            transfer_inline(self.highestBidder, self.highestBid)

        self.highestBidder = sender
        self.highestBid = value
        self.store()

    def pack(self):
        a1 = self.beneficiary
        a2 = self.auctionEnd

        # Current state of the auction.
        a3 = self.highestBidder
        a4 = self.highestBid
        a5 = self.ended
        a6 = self.locked

        return struct.pack('QQQQBB', a1, a2, a3, a4, a5, a6)

    def unpack(self, data):
        a1, a2, a3, a4, a5, a6 = struct.unpack('QQQQBB', data)
        self.beneficiary = a1
        self.auctionEnd = a2

        # Current state of the auction.
        self.highestBidder = a3
        self.highestBid = a4
        self.ended = a5
        self.locked = a6
        return self

EOS = 1397703940

def apply(receiver, code, action):
    if action == N('start'):
        print('aution start:')
        auction = SimpleAuction()
        auction.start()
    elif action == N('end'):
        auction = SimpleAuction()
        auction.end()
    elif action == N('reset'):
        auction = SimpleAuction()
        auction.reset()
    elif action == N('transfer'):
        require(code == N('eosio.token'))
        msg = read_action()
        print('transfer:', msg)
        _from, to, amount, symbol = struct.unpack('QQQQ', msg[:32])
        print('++++symbol:', symbol)
        require(symbol == EOS)
        require(to == _code)
        print('transfer', msg)
        auction = SimpleAuction()
        auction.bid(_from, amount)


pass
