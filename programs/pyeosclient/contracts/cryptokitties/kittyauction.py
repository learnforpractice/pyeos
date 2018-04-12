from backend import *
from basement import *
from kittyaccesscontrol import *
from kittybreeding import KittyBreeding
from saleclockauction import SaleClockAuction
from siringclockauction import SiringClockAuction

# @title Handles creating auctions for sale and siring of kitties.
#  This wrapper of ReverseAuction exists only so that users can create
#  auctions with only one transaction.
class KittyAuction(KittyBreeding):

    def __init__(self):
        super(KittyAuction, self).__init__()

    # @notice The auction contract variables are defined in KittyBase to allow
    #  us to refer to them in KittyOwnership to prevent accidental transfers.
    # `saleAuction` refers to the auction for gen0 and p2p sale of kitties.
    # `siringAuction` refers to the auction for siring rights of kitties.

    # @dev Sets the reference to the sale auction.
    # @param _address - Address of sale contract.
    @onlyCEO
    def setSaleAuctionAddress(self, _address: address):
        candidateContract = SaleClockAuction(_address)

        # NOTE: verify that a contract is what we expect - https:#github.com/Lunyr/crowdsale-contracts/blob/cfadd15986c30521d8ba7d5b6f57b4fefcc7ac38/contracts/LunyrToken.sol#L117
        require(candidateContract.isSaleClockAuction())

        # Set the new contract address
        self.saleAuction = candidateContract

    # @dev Sets the reference to the siring auction.
    # @param _address - Address of siring contract.
    @onlyCEO
    def setSiringAuctionAddress(self, _address: address):
        candidateContract = SiringClockAuction(_address)

        # NOTE: verify that a contract is what we expect - https://github.com/Lunyr/crowdsale-contracts/blob/cfadd15986c30521d8ba7d5b6f57b4fefcc7ac38/contracts/LunyrToken.sol#L117
        require(candidateContract.isSiringClockAuction())

        # Set the new contract address
        self.siringAuction = candidateContract;

    # @dev Put a kitty up for auction.
    #  Does some ownership trickery to create auctions in one tx.
    @whenNotPaused
    def createSaleAuction(self,
        _kittyId: uint256,
        _startingPrice: uint256,
        _endingPrice: uint256,
        _duration: uint256):
        # Auction contract checks input sizes
        # If kitty is already on any auction, this will throw
        # because it will be owned by the auction contract.
        require(self._owns(msg.sender, _kittyId));
        # Ensure the kitty is not pregnant to prevent the auction
        # contract accidentally receiving ownership of the child.
        # NOTE: the kitty IS allowed to be in a cooldown.
        require(not self.isPregnant(_kittyId));
        self._approve(_kittyId, self.saleAuction);
        # Sale auction throws if inputs are invalid and clears
        # transfer and sire approval after escrowing the kitty.
        self.saleAuction.createAuction(
            _kittyId,
            _startingPrice,
            _endingPrice,
            _duration,
            msg.sender)

    # @dev Put a kitty up for auction to be sire.
    #  Performs checks to ensure the kitty can be sired, then
    #  delegates to reverse auction.
    @whenNotPaused
    def createSiringAuction(self,
        _kittyId: uint256,
        _startingPrice: uint256,
        _endingPrice: uint256,
        _duration: uint256):
        # Auction contract checks input sizes
        # If kitty is already on any auction, this will throw
        # because it will be owned by the auction contract.
        require(self._owns(msg.sender, _kittyId))
        require(self.isReadyToBreed(_kittyId))
        self._approve(_kittyId, self.siringAuction)
        # Siring auction throws if inputs are invalid and clears
        # transfer and sire approval after escrowing the kitty.
        self.siringAuction.createAuction(
            _kittyId,
            _startingPrice,
            _endingPrice,
            _duration,
            msg.sender
        )

    # @dev Completes a siring auction by bidding.
    #  Immediately breeds the winning matron with the sire on auction.
    # @param _sireId - ID of the sire on auction.
    # @param _matronId - ID of the matron owned by the bidder.
    @whenNotPaused
    def bidOnSiringAuction(self, _sireId: uint256, _matronId: uint256):
        # Auction contract checks input sizes
        require(self._owns(msg.sender, _matronId));
        require(self.isReadyToBreed(_matronId));
        require(self._canBreedWithViaAuction(_matronId, _sireId));

        # Define the current price of the auction.
        self.currentPrice = self.siringAuction.getCurrentPrice(_sireId);
        require(msg.value >= self.currentPrice + self.autoBirthFee);

        # Siring auction will throw if the bid fails.
        self.siringAuction.bid.value(msg.value - self.autoBirthFee)(_sireId);
        self._breedWith(uint32(_matronId), uint32(_sireId));

    # @dev Transfers the balance of the sale auction contract
    # to the KittyCore contract. We use two-step withdrawal to
    # prevent two transfer calls in the auction bid function.
    @onlyCLevel
    def withdrawAuctionBalances(self):
        self.saleAuction.withdrawBalance()
        self.siringAuction.withdrawBalance()

