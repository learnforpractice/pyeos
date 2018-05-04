#porting from http://solidity.readthedocs.io/en/latest/solidity-by-example.html
import ustruct as struct
from eoslib import *
from backyard.solidity import *
from backyard.storage import SDict, SList, storage_find, storage_get, storage_set, _get_hash, _pack, _unpack
from backyard.token import transfer

'''
    function SimpleAuction(
        uint _biddingTime,
        address _beneficiary
    ) public {
        beneficiary = _beneficiary;
        auctionEnd = now + _biddingTime;
    }
'''

code = N('auction1')

class SimpleAuction(object):
    # The following is a so-called natspec comment,
    # recognizable by the three slashes.
    # It will be shown when the user is asked to
    # confirm a transaction.

    # Create a simple auction with `_biddingTime`
    # seconds bidding time on behalf of the
    # beneficiary address `_beneficiary`.
    def __init__(self):
    # Parameters of the auction. Times are either
    # absolute unix timestamps (seconds since 1970-01-01)
    # or time periods in seconds.
        self._table_id = N('auction')
        self.beneficiary = 0 #address(0)
        self.auctionEnd = 0

        # Current state of the auction.
        self.highestBidder = 0
        self.highestBid = 0

        # Allowed withdrawals of previous bids
#        mapping(address => uint) pendingReturns;
        self.pendingReturns = SDict(code, table_id=1)
        # Set to true at the end, disallows any change
        self.ended = 0
        
        self.load()

    def start(self):
        _msg = read_action()
        require(len(_msg) == 16)
        self.beneficiary = int.from_bytes(_msg[8:], 'little')
        self.auctionEnd = now() + int.from_bytes(_msg[:8], 'little')
        self.save()

    @property
    def beneficiary(self):
        id = _get_hash('beneficiary')
        value = storage_get(code, self._table_id, id)
        value = _unpack(value)
        return value

    @beneficiary.setter
    def beneficiary(self, value):
        self._beneficiary = value
        id = _get_hash('beneficiary')
        value = _pack(value)
        storage_set(code, self._table_id, id, value)

    @property
    def auctionEnd(self):
        id = _get_hash('auctionEnd')
        value = storage_get(code, self._table_id, id)
        value = _unpack(value)
        return value

    @auctionEnd.setter
    def auctionEnd(self, value):
        self._auctionEnd = value
        id = _get_hash('auctionEnd')
        value = _pack(value)
        storage_set(code, self._table_id, id, value)

    # Events that will be fired on changes.
    @event 
    def HighestBidIncreased(self, bidder: address, amount: int):
        pass

    @event 
    def AuctionEnded(self, winner: address, amount: int):
        pass

    # Bid on the auction with the value sent
    # together with this transaction.
    # The value will only be refunded if the
    # auction is not won.
    @payable
    def bid(self, sender, value):
        # No arguments are necessary, all
        # information is already part of
        # the transaction. The keyword payable
        # is required for the function to
        # be able to receive Ether.

        # Revert the call if the bidding
        # period is over.
        require(
            now() <= self.auctionEnd,
            "Auction already ended."
        );

        # If the bid is not higher, send the
        # money back.
        require(
            value > self.highestBid,
            "There already is a higher bid."
        );

        if self.highestBid != 0:
            # Sending back the money by simply using
            # highestBidder.send(highestBid) is a security risk
            # because it could execute an untrusted contract.
            # It is always safer to let the recipients
            # withdraw their money themselves.
            self.pendingReturns[self.highestBidder] += self.highestBid;

        self.highestBidder = sender;
        self.highestBid = value;
        self.HighestBidIncreased(sender, value);

    # Withdraw a bid that was overbid.
    def withdraw(self, sender) -> bool:
        '''
        if not self.pendingReturns.find(sender):
            return False
        amount = self.pendingReturns[sender]
        if amount < 0:
            return False
        # It is important to set this to zero because the recipient
        # can call this function again as part of the receiving call
        # before `send` returns.
        self.pendingReturns[sender] = 0
        '''
        
        auth = struct.pack('QQ', sender, N('active'))
        t = transfer()
        t._from = sender
        t.to = sender
#        t.amount = amount
        t.amount = 100
        t.precision = 4
        t.symbol = 'EOS'
        t.memo = 'hello,world'
        data = t.pack()
        print(data)
        t.unpack(data)
        t.p()

        print('before require_auth')
#        require_auth( sender )
#        require_auth( code )
        print('hello, world')
#        send_inline( N('eosio.token'), N('transfer'), auth, data)
        print('+++++++++++ call transfer.')
        #multi_index.hpp 695: cannot modify objects in table of another contract
        wasm_call(N('eosio.token'), 'apply', N('eosio.token'), N('eosio.token'), N('transfer'))

        #send_inline is asynchronized, 
        #it's imposible to check the result of send_inline
        #if send_inline failed, 
        #it will throw an exception,
        #all the changes made in the transaction will be rewind
        self.pendingReturns[sender] = amount
        return True

        '''
        if not sender.send(amount):
            # No need to call throw here, just reset the amount owing
            self.pendingReturns[sender] = amount
            return False;
        '''

    # End the auction and send the highest bid
    # to the beneficiary.
    def auctionEnd(self):
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
        self.AuctionEnded(self.highestBidder, self.highestBid)

        # 3. Interaction
        #FIXME
        self.beneficiary.transfer(self.highestBid)
        self.save()

    def load(self):
        for key in self.__dict__:
            if key[0] == '_' or key == 'pendingReturns':
                continue
            id = _get_hash(key)
            value = storage_find(code, self._table_id, id)
            if value:
                value = _unpack(value)
                setattr(self, key, value)

    def save(self):
        for key in self.__dict__:
            if key[0] == '_' or key == 'pendingReturns':
                continue
            id = _get_hash(key)
            value = self.__dict__[key]
            value = _pack(value)
            storage_set(code, self._table_id, id, value)

def apply(name, type):
    if type == N('init'):
        _msg = read_action()
        require(len(_msg) == 16)
        msg = Msg()
        msg.sender = int.from_bytes(_msg[:8], 'little')
        msg.value = int.from_bytes(_msg[8:], 'little')

        auction = SimpleAuction()
        auction.init()
    elif type == N('start'):
        auction = SimpleAuction()
        auction.start()
    elif type == N('withdraw'):
        _msg = read_action()
#        require(len(_msg) == 8)
        sender = int.from_bytes(_msg[8:16], 'little')
        auction = SimpleAuction()
        auction.withdraw(sender)
    elif type == N('transfer'):
        msg = read_action()
        print('transfer', msg)
        t = transfer()
        t.unpack(msg)
        t.p()
        auction = SimpleAuction()
        auction.bid(t._from, t.amount)


