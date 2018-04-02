#Represents an auction on an NFT
from backend import *
class Auction:
    def __init__(self):
        self.seller = address(0)
        #Price (in wei) at beginning of auction
        self.startingPrice = uint128(0)
        #Price (in wei) at end of auction
        self.endingPrice = uint128(0)
        #Duration (in seconds) of auction
        self.duration = uint64(0)
        #Time when auction started
        #NOTE: 0 if this auction has been concluded
        self.startedAt = uint64(0)

    #FIXME
    def pack(self):
        pass

    def unpack(self):
        pass

    '''
    #Current owner of NFT
    address seller;
    #Price (in wei) at beginning of auction
    uint128 startingPrice;
    #Price (in wei) at end of auction
    uint128 endingPrice;
    #Duration (in seconds) of auction
    uint64 duration;
    #Time when auction started
    #NOTE: 0 if this auction has been concluded
    uint64 startedAt;
    '''

