from backend import *
from basement import *
from backyard.storage import SList, SDict
from kitty import Kitty

#from kittyaccesscontrol import KittyAccessControl
from kittyaccesscontrol import *

# @title Base contract for CryptoKitties. Holds all common structs, events and base variables.
# @author Axiom Zen (https://www.axiomzen.co)
# @dev See the KittyCore contract documentation to understand how the various contract facets are arranged.
class KittyBase(KittyAccessControl):
    #/*** EVENTS ***/
    def __init__(self):
        super(KittyBase, self).__init__()
        '''FIXME:
        self.cooldowns = [
            uint32(1 minutes),
            uint32(2 minutes),
            uint32(5 minutes),
            uint32(10 minutes),
            uint32(30 minutes),
            uint32(1 hours),
            uint32(2 hours),
            uint32(4 hours),
            uint32(8 hours),
            uint32(16 hours),
            uint32(1 days),
            uint32(2 days),
            uint32(4 days),
            uint32(7 days) ]
        '''
        # An approximation of currently how many seconds are in between blocks.
        self.secondsPerBlock = uint256(15)
    
        #/*** STORAGE ***/
    
        # @dev An array containing the Kitty struct for all Kitties in existence. The ID
        #  of each cat is actually an index into this array. Note that ID 0 is a negacat,
        #  the unKitty, the mythical beast that is the parent of all gen0 cats. A bizarre
        #  creature that is both matron and sire... to itself! Has an invalid genetic code.
        #  In other words, cat ID 0 is invalid... ;-)
        #Kitty[] kitties;
        self.kitties = SList(0)
    
        # @dev A mapping from cat IDs to the address that owns them. All cats have
        #  some valid owner address, even gen0 cats are created with a non-zero owner.
        #mapping (uint256 => address) public kittyIndexToOwner;
        self.kittyIndexToOwner = SList(0, value_type = Kitty)
    
        # @dev A mapping from owner address to count of tokens that address owns.
        #  Used internally inside balanceOf() to resolve ownership count.
        #mapping (address => uint256) ownershipTokenCount;
        self.ownershipTokenCount = SDict(0)
    
        # @dev A mapping from KittyIDs to an address that has been approved to call
        #  transferFrom(). Each Kitty can only have one approved address for transfer
        #  at any time. A zero value means no approval is outstanding.
        #mapping (uint256 => address) public kittyIndexToApproved;
        self.kittyIndexToApproved = SDict(0)
    
        # @dev A mapping from KittyIDs to an address that has been approved to use
        #  this Kitty for siring via breedWith(). Each Kitty can only have one approved
        #  address for siring at any time. A zero value means no approval is outstanding.
        #mapping (uint256 => address) public sireAllowedToAddress;
        self.sireAllowedToAddress = SDict(0)
    
        # @dev The address of the ClockAuction contract that handles sales of Kitties. This
        #  same contract handles both peer-to-peer sales as well as the gen0 sales which are
        #  initiated every 15 minutes.
        #SaleClockAuction public saleAuction;
        self.saleAuction = None
    
        # @dev The address of a custom ClockAuction subclassed contract that handles siring
        #  auctions. Needs to be separate from saleAuction because the actions taken on success
        #  after a sales and siring auction are quite different.
        #SiringClockAuction public siringAuction;
        self.siringAuction = None
    # @dev The Birth event is fired whenever a new kitten comes into existence. This obviously
    #  includes any time a cat is created through the giveBirth method, but it is also called
    #  when a new gen0 cat is created.
    #event Birth(address owner, uint256 kittyId, uint256 matronId, uint256 sireId, uint256 genes);
    @event
    def Birth(self, owner: address, kittyId: uint256, matronId: uint256, sireId: uint256, genes: uint256):
        pass

    # @dev Transfer event as defined in current draft of ERC721. Emitted every time a kitten
    #  ownership is assigned, including births.
    #event Transfer(address from, address to, uint256 tokenId);
    @event
    def Transfer(self, _from: address, to: address, tokenId: uint256):
        pass

    #/*** CONSTANTS ***/

    # @dev A lookup table indicating the cooldown duration after any successful
    #  breeding action, called "pregnancy time" for matrons and "siring cooldown"
    #  for sires. Designed such that the cooldown roughly doubles each time a cat
    #  is bred, encouraging owners not to just keep breeding the same cat over
    #  and over again. Caps out at one week (a cat can breed an unbounded number
    #  of times, and the maximum cooldown is always seven days).


    # @dev Assigns ownership of a specific Kitty to an address.
    def _transfer(self, _from: address, _to: address, _tokenId: uint256):
        # Since the number of kittens is capped to 2^32 we can't overflow this
        try:
            self.ownershipTokenCount[_to]+=1
        except KeyError:
            self.ownershipTokenCount[_to] = 1

        # transfer ownership
        self.kittyIndexToOwner[_tokenId] = _to
        # When creating new kittens _from is 0x0, but we can't account that address.
        if _from != address(0):
            self.ownershipTokenCount[_from]-=1
            # once the kitten is transferred also clear sire allowances
            del self.sireAllowedToAddress[_tokenId]
            # clear any previously approved ownership exchange
            del self.kittyIndexToApproved[_tokenId]

        # Emit the transfer event.
        self.Transfer(_from, _to, _tokenId)

    # @dev An internal method that creates a new kitty and stores it. This
    #  method doesn't do any checking and should only be called when the
    #  input data is known to be valid. Will generate both a Birth event
    #  and a Transfer event.
    # @param _matronId The kitty ID of the matron of this cat (zero for gen0)
    # @param _sireId The kitty ID of the sire of this cat (zero for gen0)
    # @param _generation The generation number of this cat, must be computed by caller.
    # @param _genes The kitty's genetic code.
    # @param _owner The inital owner of this cat, must be non-zero (except for the unKitty, ID 0)
    def _createKitty(self, _matronId: uint256, _sireId: uint256, _generation: uint256, 
                     _genes: uint256, _owner: address) -> uint:
        # These requires are not strictly necessary, our calling code should make
        # sure that these conditions are never broken. However! _createKitty() is already
        # an expensive call (for storage), and it doesn't hurt to be especially careful
        # to ensure our data structures are always valid.
        require(_matronId == uint256(uint32(_matronId)))
        require(_sireId == uint256(uint32(_sireId)))
        require(_generation == uint256(uint16(_generation)))

        # New kitty starts with the same cooldown as parent gen/2
        cooldownIndex = uint16(_generation / 2)
        if cooldownIndex > 13:
            cooldownIndex = 13

        _kitty = Kitty(
            _genes = _genes,
            _birthTime = uint64(now()),
            _cooldownEndBlock = uint64(0),
            _matronId = uint32(_matronId),
            _sireId = uint32(_sireId),
            _siringWithId = uint32(0),
            _cooldownIndex = cooldownIndex,
            _generation = uint16(_generation) )

        newKittenId = self.kitties.push(_kitty) - 1

        # It's probably never going to happen, 4 billion cats is A LOT, but
        # let's just be 100% sure we never let this happen.
        require(newKittenId == uint256(uint32(newKittenId)))

        # emit the birth event
        self.Birth(
            _owner,
            newKittenId,
            uint256(_kitty.matronId),
            uint256(_kitty.sireId),
            _kitty.genes
        )

        # This will assign ownership, and also emit the Transfer event as
        # per ERC721 draft
        self._transfer(0, _owner, newKittenId)
        return newKittenId

    # Any C-level can fix how many seconds per blocks are currently observed.
    def setSecondsPerBlock(self, secs: uint256):
        self.onlyCLevel()
        require(secs < self.cooldowns[0])
        self.secondsPerBlock = secs


