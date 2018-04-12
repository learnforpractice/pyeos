from backend import *
from basement import *
#from pausable import Pausable
from pausable import *
from clockauctionbase import ClockAuctionBase
from erc721 import ERC721
from auction import Auction

# @title Clock auction for non-fungible tokens.
# @notice We omit a fallback function to prevent accidental sends to this contract.
class ClockAuction(Pausable, ClockAuctionBase):
    # @dev The ERC-165 interface signature for ERC-721.
    #  Ref: https://github.com/ethereum/EIPs/issues/165
    #  Ref: https://github.com/ethereum/EIPs/issues/721
    
    # @dev Constructor creates a reference to the NFT ownership contract
    #  and verifies the owner cut is in the valid range.
    # @param _nftAddress - address of a deployed contract implementing
    #  the Nonfungible Interface.
    # @param _cut - percent cut the owner takes on each auction, must be
    #  between 0-10,000.
    def __init__(self, _nftAddress: address, _cut: uint256):
        require(_cut <= 10000)

        Pausable.__init__(self)
        ClockAuctionBase.__init__(self)
        self.InterfaceSignature_ERC721 = bytes4(b'\x9a\x20\x48\x3d')

        self.ownerCut = _cut;

        candidateContract = ERC721(_nftAddress)
        #FIXME:
        require(candidateContract.supportsInterface(self.InterfaceSignature_ERC721))
        self.nonFungibleContract = candidateContract


    # @dev Remove all Ether from the contract, which is the owner's cuts
    #  as well as any Ether sent directly to the contract address.
    #  Always transfers to the NFT contract, but can be called either by
    #  the owner or the NFT contract.
    def withdrawBalance(self):
        nftAddress = address(self.nonFungibleContract)
        require(msg.sender == self.owner or msg.sender == nftAddress)
        # We are using this boolean method to make sure that even if one fails it will still work
        res = nftAddress.send(this.balance)

    # @dev Creates and begins a new auction.
    # @param _tokenId - ID of token to auction, sender must be owner.
    # @param _startingPrice - Price of item (in wei) at beginning of auction.
    # @param _endingPrice - Price of item (in wei) at end of auction.
    # @param _duration - Length of time to move between starting
    #  price and ending price (in seconds).
    # @param _seller - Seller, if not the message sender
    @whenNotPaused
    def createAuction(self, _tokenId: uint256, _startingPrice: uint256,
                           _endingPrice: uint256,
                            _duration: uint256,
                            _seller: uint256):
        # Sanity check that no inputs overflow how many bits we've allocated
        # to store them in the auction struct.
        require(_startingPrice == uint256(uint128(_startingPrice)))
        require(_endingPrice == uint256(uint128(_endingPrice)))
        require(_duration == uint256(uint64(_duration)))

        require(self._owns(msg.sender, _tokenId))
        self._escrow(msg.sender, _tokenId)
        auction = Auction(_seller, uint128(_startingPrice), uint128(_endingPrice), 
                          uint64(_duration),
                          uint64(now()))
        self._addAuction(_tokenId, auction)

    # @dev Bids on an open auction, completing the auction and transferring
    #  ownership of the NFT if enough Ether is supplied.
    # @param _tokenId - ID of token to bid on.
    @whenNotPaused
    def bid(self, _tokenId: uint256):
        # _bid will throw if the bid or funds transfer fails
        self._bid(_tokenId, msg.value);
        self._transfer(msg.sender, _tokenId);

    # @dev Cancels an auction that hasn't been won yet.
    #  Returns the NFT to original owner.
    # @notice This is a state-modifying function that can
    #  be called while the contract is paused.
    # @param _tokenId - ID of token on auction
    def cancelAuction(self, _tokenId: uint256):
        auction = self.tokenIdToAuction[_tokenId]
        require(self._isOnAuction(auction))
        
        seller = auction.seller
        require(msg.sender == seller)
        self._cancelAuction(_tokenId, seller)

    # @dev Cancels an auction when the contract is paused.
    #  Only the owner may do this, and NFTs are returned to
    #  the seller. This should only be used in emergencies.
    # @param _tokenId - ID of the NFT on auction to cancel.
    @whenPaused
    @onlyOwner
    def cancelAuctionWhenPaused(self, _tokenId: uint256):
        auction = self.tokenIdToAuction[_tokenId]
        require(self._isOnAuction(auction))
        self._cancelAuction(_tokenId, auction.seller)

    # @dev Returns auction info for an NFT on auction.
    # @param _tokenId - ID of NFT on auction.
    def getAuction(self, _tokenId: uint256):
        auction = self.tokenIdToAuction[_tokenId]
        require(self._isOnAuction(auction))
        return (
            auction.seller,
            auction.startingPrice,
            auction.endingPrice,
            auction.duration,
            auction.startedAt)

    # @dev Returns the current price of an auction.
    # @param _tokenId - ID of the token price we are checking.
    def getCurrentPrice(self, _tokenId: uint256) -> uint256:
        auction = self.tokenIdToAuction[_tokenId]
        require(self._isOnAuction(auction))
        return self._currentPrice(auction)
