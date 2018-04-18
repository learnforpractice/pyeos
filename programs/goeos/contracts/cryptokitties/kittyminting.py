from backend import *
from basement import *
from kittyaccesscontrol import *

from kittyauction import KittyAuction
# @title all functions related to creating kittens
class KittyMinting(KittyAuction):
    def __init__(self):
        super(KittyMinting, self).__init__()
    # Limits the number of cats the contract owner can ever create.
        '''FIXME: constant
        uint256 public constant PROMO_CREATION_LIMIT = 5000;
        uint256 public constant GEN0_CREATION_LIMIT = 45000;
    
        # Constants for gen0 auctions.
        uint256 public constant GEN0_STARTING_PRICE = 10 finney;
        uint256 public constant GEN0_AUCTION_DURATION = 1 days;
        '''
        self.PROMO_CREATION_LIMIT = 5000;
        self.GEN0_CREATION_LIMIT = 45000;
        # Constants for gen0 auctions.
        #FIXME: 
        self.GEN0_STARTING_PRICE = 10 #finney;
        self.GEN0_AUCTION_DURATION = 1 #days;

    # Counts the number of cats the contract owner has created.
        self.promoCreatedCount = uint256(0)
        self.en0CreatedCount = uint256(0)
    # @dev we can create promo kittens, up to a limit. Only callable by COO
    # @param _genes the encoded genes of the kitten to be created, any value is accepted
    # @param _owner the future owner of the created kittens. Default to contract COO
    @onlyCOO
    def createPromoKitty(self, _genes: uint256, _owner: address):
        kittyOwner = _owner;
        if kittyOwner == address(0):
             kittyOwner = self.cooAddress
        require(self.promoCreatedCount < self.PROMO_CREATION_LIMIT)
        self.promoCreatedCount+=1
        self._createKitty(0, 0, 0, _genes, kittyOwner)

    # @dev Creates a new gen0 kitty with the given genes and
    #  creates an auction for it.
    @onlyCOO
    def createGen0Auction(self, _genes: uint256):
        require(self.gen0CreatedCount < self.GEN0_CREATION_LIMIT)

        kittyId = self._createKitty(0, 0, 0, _genes, address(this))
        self._approve(kittyId, self.saleAuction)

        self.saleAuction.createAuction(
            kittyId,
            self._computeNextGen0Price(),
            0,
            self.GEN0_AUCTION_DURATION,
            address(this) )
        self.gen0CreatedCount+=1

    # @dev Computes the next gen0 auction starting price, given
    #  the average of the past 5 prices + 50%.
    def _computeNextGen0Price(self) -> uint256:
        avePrice = self.saleAuction.averageGen0SalePrice()

        # Sanity check to ensure we don't overflow arithmetic
        require(avePrice == uint256(uint128(avePrice)))

        nextPrice = avePrice + (avePrice / 2)

        # We never auction for less than starting price
        if nextPrice < self.GEN0_STARTING_PRICE:
            nextPrice = self.GEN0_STARTING_PRICE;

        return nextPrice;
