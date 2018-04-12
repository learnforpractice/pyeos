from backend import *
from basement import *
from auction import Auction
from erc721 import ERC721
from storage import SDict
# @title Auction Core
# @dev Contains models, variables, and internal methods for the auction.
# @notice We omit a fallback function to prevent accidental sends to this contract.
class ClockAuctionBase:
    def __init__(self):
        #Reference to contract tracking NFT ownership
        #ERC721 public nonFungibleContract;
        self.nonFungibleContract = ERC721()
        #Cut owner takes on each auction, measured in basis points (1/100 of a percent).
        #Values 0-10,000 map to 0%-100%
        #uint256 public ownerCut;
        self.ownerCut = uint256(0)
    
        #Map from token ID to their corresponding auction.
        #mapping (uint256 => Auction) tokenIdToAuction;
        self.tokenIdToAuction = SDict(key_type = uint256, value_type = Auction)


    @event
    def AuctionCreated(self, tokenId: uint256, startingPrice: uint256, endingPrice: uint256, duration: uint256): pass

    @event
    def AuctionSuccessful(self, tokenId: uint256, totalPrice: uint256, winner: address): pass
    
    @event
    def AuctionCancelled(self, tokenId: uint256): pass


    # @dev Returns true if the claimant owns the token.
    # @param _claimant - Address claiming to own the token.
    # @param _tokenId - ID of token whose ownership to verify.
    def _owns(self, _claimant: address, _tokenId: uint256) -> bool:
        return self.nonFungibleContract.ownerOf(_tokenId) == _claimant

    # @dev Escrows the NFT, assigning ownership to this contract.
    # Throws if the escrow fails.
    # @param _owner - Current owner address of token to escrow.
    # @param _tokenId - ID of token whose approval to verify.
    def _escrow(self, _owner: address, _tokenId: uint256):
        #it will throw if transfer fails
        #FIXME this
        self.nonFungibleContract.transferFrom(_owner, this, _tokenId)

    # @dev Transfers an NFT owned by this contract to another address.
    # Returns true if the transfer succeeds.
    # @param _receiver - Address to transfer NFT to.
    # @param _tokenId - ID of token to transfer.
    def _transfer(self, _receiver: address, _tokenId: uint256):
        #it will throw if transfer fails
        self.nonFungibleContract.transfer(_receiver, _tokenId)

    # @dev Adds an auction to the list of open auctions. Also fires the
    #  AuctionCreated event.
    # @param _tokenId The ID of the token to be put on auction.
    # @param _auction Auction to add.
    def _addAuction(self, _tokenId: uint256, _auction: Auction):
        #Require that all auctions have a duration of
        #at least one minute. (Keeps our math from getting hairy!)
#        require(_auction.duration >= 1 minutes)
        require(_auction.duration >= 60)
        self.tokenIdToAuction[_tokenId] = _auction;

        self.AuctionCreated(
            uint256(_tokenId),
            uint256(_auction.startingPrice),
            uint256(_auction.endingPrice),
            uint256(_auction.duration)
        )

    # @dev Cancels an auction unconditionally.
    def _cancelAuction(self, _tokenId: uint256, _seller: address):
        self._removeAuction(_tokenId)
        self._transfer(_seller, _tokenId)
        self.AuctionCancelled(_tokenId)

    # @dev Computes the price and transfers winnings.
    # Does NOT transfer ownership of token.
    def _bid(self, _tokenId: uint256, _bidAmount: uint256) -> uint256:
        #Get a reference to the auction struct
        auction = self.tokenIdToAuction[_tokenId];

        #Explicitly check that this auction is currently live.
        #(Because of how Ethereum mappings work, we can't just count
        #on the lookup above failing. An invalid _tokenId will just
        #return an auction object that is all zeros.)
        require(self._isOnAuction(auction));

        #Check that the bid is greater than or equal to the current price
        price = self._currentPrice(auction);
        require(_bidAmount >= price);

        #Grab a reference to the seller before the auction struct
        #gets deleted.
        seller = auction.seller;

        #The bid is good! Remove the auction before sending the fees
        #to the sender so we can't have a reentrancy attack.
        self._removeAuction(_tokenId);

        #Transfer proceeds to seller (if there are any!)
        if price > 0:
            #Calculate the auctioneer's cut.
            #(NOTE: _computeCut() is guaranteed to return a
            #value <= price, so this subtraction can't go negative.)
            auctioneerCut = self._computeCut(price);
            sellerProceeds = price - auctioneerCut;

            #NOTE: Doing a transfer() in the middle of a complex
            #method like this is generally discouraged because of
            #reentrancy attacks and DoS attacks if the seller is
            #a contract with an invalid fallback function. We explicitly
            #guard against reentrancy attacks by removing the auction
            #before calling transfer(), and the only thing the seller
            #can DoS is the sale of their own asset! (And if it's an
            #accident, they can call cancelAuction(). )
            seller.transfer(sellerProceeds)

        #Calculate any excess funds included with the bid. If the excess
        #is anything worth worrying about, transfer it back to bidder.
        #NOTE: We checked above that the bid amount is greater than or
        #equal to the price so this cannot underflow.
        bidExcess = _bidAmount - price

        #Return the funds. Similar to the previous transfer, this is
        #not susceptible to a re-entry attack because the auction is
        #removed before any transfers occur.
        msg.sender.transfer(bidExcess)

        #Tell the world!
        self.AuctionSuccessful(_tokenId, price, msg.sender)

        return price;

    # @dev Removes an auction from the list of open auctions.
    # @param _tokenId - ID of NFT on auction.
    def _removeAuction(self, _tokenId: uint256):
        del self.tokenIdToAuction[_tokenId]

    # @dev Returns true if the NFT is on auction.
    # @param _auction - Auction to check.
    def _isOnAuction(self, _auction: Auction) -> bool:
        return _auction.startedAt > 0

    # @dev Returns current price of an NFT on auction. Broken into two
    #  functions (this one, that computes the duration from the auction
    #  structure, and the other that does the price computation) so we
    #  can easily test that the price computation works correctly.
    def _currentPrice(self, _auction: Auction) -> uint256:
        self.secondsPassed = 0;
        #A bit of insurance against negative values (or wraparound).
        #Probably not necessary (since Ethereum guarnatees that the
        #now variable doesn't ever go backwards).
        if now() > _auction.startedAt:
            self.secondsPassed = now() - _auction.startedAt;

        return self._computeCurrentPrice(
            _auction.startingPrice,
            _auction.endingPrice,
            _auction.duration,
            self.secondsPassed
        )

    # @dev Computes the current price of an auction. Factored out
    #  from _currentPrice so we can run extensive unit tests.
    #  When testing, make this function public and turn on
    #  `Current price computation` test suite.
    def _computeCurrentPrice(self, _startingPrice: uint256, _endingPrice: uint256, _duration: uint256, _secondsPassed: uint256) -> uint256:
        #NOTE: We don't use SafeMath (or similar) in this function because
        # all of our public functions carefully cap the maximum values for
        # time (at 64-bits) and currency (at 128-bits). _duration is
        # also known to be non-zero (see the require() statement in
        # _addAuction())
        if _secondsPassed >= _duration:
            #We've reached the end of the dynamic pricing portion
            #of the auction, just return the end price.
            return _endingPrice;
        else:
            #Starting price can be higher than ending price (and often is!), so
            #this delta can be negative.
            self.totalPriceChange = int256(_endingPrice) - int256(_startingPrice);

            #This multiplication can't overflow, _secondsPassed will easily fit within
            #64-bits, and totalPriceChange will easily fit within 128-bits, their product
            #will always fit within 256-bits.
            self.currentPriceChange = self.totalPriceChange * int256(_secondsPassed) / int256(_duration);

            #currentPriceChange can be negative, but if so, will have a magnitude
            #less that _startingPrice. Thus, this result will always end up positive.
            self.currentPrice = int256(_startingPrice) + self.currentPriceChange;

            return uint256(self.currentPrice);

    # @dev Computes owner's cut of a sale.
    # @param _price - Sale price of NFT.
    def _computeCut(self, _price: uint256)  -> uint256:
        #NOTE: We don't use SafeMath (or similar) in this function because
        # all of our entry functions carefully cap the maximum values for
        # currency (at 128-bits), and ownerCut <= 10000 (see the require()
        # statement in the ClockAuction constructor). The result of this
        # function is always guaranteed to be <= _price.
        return _price * self.ownerCut / 10000;
