from backend import *
from basement import *
from auction import Auction
from clockauction import ClockAuction
# @title Clock auction modified for sale of kitties
# @notice We omit a fallback function to prevent accidental sends to this contract.
class SaleClockAuction(ClockAuction):
    # Delegate constructor
    def __init__(self, _nftAddr: address, _cut: uint256):
        ClockAuction.__init__(self, _nftAddr, _cut)
        # @dev Sanity check that allows us to ensure that we are pointing to the
        #  right auction in our setSaleAuctionAddress() call.
        self.isSaleClockAuction = True
        
        # Tracks last 5 sale price of gen0 kitty sales
        gen0SaleCount = uint256(0)
        lastGen0SalePrices = List(size = 5, value_type = uint256)

    # @dev Creates and begins a new auction.
    # @param _tokenId - ID of token to auction, sender must be owner.
    # @param _startingPrice - Price of item (in wei) at beginning of auction.
    # @param _endingPrice - Price of item (in wei) at end of auction.
    # @param _duration - Length of auction (in seconds).
    # @param _seller - Seller, if not the message sender
    def createAuction(self,
        _tokenId: uint256,
        _startingPrice: uint256,
        _endingPrice: uint256,
        _duration: uint256,
        _seller: address):
        # Sanity check that no inputs overflow how many bits we've allocated
        # to store them in the auction struct.
        require(_startingPrice == uint256(uint128(_startingPrice)))
        require(_endingPrice == uint256(uint128(_endingPrice)))
        require(_duration == uint256(uint64(_duration)))

        require(msg.sender == address(self.nonFungibleContract))
        self._escrow(_seller, _tokenId);
        auction = Auction(
            _seller,
            uint128(_startingPrice),
            uint128(_endingPrice),
            uint64(_duration),
            uint64(now)
        )
        self._addAuction(_tokenId, auction)

    # @dev Updates lastSalePrice if seller is the nft contract
    # Otherwise, works the same as default bid method.
    def bid(self, _tokenId: uint256):
        # _bid verifies token ID size
        seller = self.tokenIdToAuction[_tokenId].seller
        price = self._bid(_tokenId, msg.value)
        self._transfer(msg.sender, _tokenId)

        # If not a gen0 auction, exit
        if seller == address(self.nonFungibleContract):
            # Track gen0 sale prices
            self.lastGen0SalePrices[self.gen0SaleCount % 5] = price;
            self.gen0SaleCount+=1;

    def averageGen0SalePrice(self) -> uint256:
        sum = uint256(0)
        for i in range(5): 
            sum += self.lastGen0SalePrices[i];
        return uint256(sum / 5)
