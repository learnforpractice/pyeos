from eoslib import *
'''
msg.data (bytes): complete calldata
msg.gas (uint): remaining gas - deprecated in version 0.4.21 and to be replaced by gasleft()
msg.sender (address): sender of the message (current call)
msg.sig (bytes4): first four bytes of the calldata (i.e. function identifier)
msg.value (uint): number of wei sent with the message
'''

contract_owner = N('cryptokittie') #sorry kitty, max 12 charactor support

def balanceOf(address):
    return 99999

def require(condition, msg = ''):
    eosio_assert(condition)

class message:
    def __init__(self, data, gas, sender, sig, value):
        self.data = data
        self.gas = gas
        self.sender = sender
        self.sig = sig
        self.value = value
#msg.sender.send

'''
block.blockhash(uint blockNumber) returns (bytes32): hash of the given block - only works for 256 most recent blocks excluding current
block.coinbase (address): current block miner’s address
block.difficulty (uint): current block difficulty
block.gaslimit (uint): current block gaslimit
block.number (uint): current block number
block.timestamp (uint): current block timestamp as seconds since unix epoch
'''

class block:
    blockhash = None
    coinbase = None
    difficulty = None
    gaslimit = None
    number = None
    timestamp = None

def keccak256(_str):
    return sha256(_str)

class bytes4(object):
    def __init__(self, value):
        if isinstance(value, str):
            self._v = bytes(value, 'utf8')
        elif isinstance(value, bytes):
            self._v = value
        else:
            raise Exception("unsupported type")

class bytes32(bytes):
    pass

class List(object):
    def __init__(self, size = 0, value = [], value_type = object):
        if value:
            self._list = value
        else:
            self._list = [None for i in range(size)]
        '''
        eosio_assert(isinstance(value, list), "bad type")
        self._list = value
        self.value_type = value_type
        '''
        
    def __getitem__(self, index):
        return self._list[index]

    def __setitem__(self, index, val):
        eosio_assert(isinstance(val, self.value_type), 'bad value type')
        self._list[index] = val

    def push(self, value):
        self._list.append(value)
        return len(self._list)

    @property
    def length(self):
        return len(self._list)

class Dict(object):
    def __init__(self, value = {}, key_type = object, value_type = object):
        eosio_assert(isinstance(value, list), "bad type")
        self._dict = value
        self.key_type = key_type
        self.value_type = value_type

    def __getitem__(self, key):
        eosio_assert(isinstance(key, self.key_type), 'bad key type')
        return self._dict[key]

    def __setitem__(self, key, val):
        eosio_assert(isinstance(key, self.key_type), 'bad key type')
        eosio_assert(isinstance(val, self.value_type), 'bad value type')

        self._list[key] = val

class uint16(int):
    pass

class uint(int):
    pass

class uint32(int):
    pass

class uint64(int):
    pass

class uint128(int):
    pass


class uint256(object):
    def __init__(self, v):
        assert isinstance(v, int)
        self._v = v

    def __add__(self, other):
        if isinstance(other, int):
            self._v += other
        else:
            self._v += other._v
        return self.__class__(self._v)

    def __sub__(self, other):
        if isinstance(other, int):
            self._v -= other
        else:
            self._v -= other._v
        return self.__class__(self._v)

    def __mul__(self, other):
        if isinstance(other, int):
            self._v *= other
        else:
            self._v *= other._v
        return self.__class__(self._v)

    def __div__(self, other):
        if isinstance(other, int):
            self._v /= other
        else:
            self._v /= other._v
        return self.__class__(self._v)

    def __truediv__(self,other):
        if isinstance(other, int):
            self._v = int(self._v / other)
        else:
            self._v = int(self._v / other._v)
        return self.__class__(self._v)

    def __repr__(self):
        return '%s(%s)' % (type(self).__name__, str(self._v))

class address(int):
    def __init__(self):
        self.balance = 0
    
    def transfer(self, _from, _to):
        pass
    def ownerOf(self,address):
        pass

this = address(991)

msg = message('',0,'110',0,0)

class Ownable:
    def __init__(self):
        self.owner = None
        itr = db_find_i64(contract_owner, contract_owner, contract_owner, N('owner'))
        if itr >= 0:
            value = db_get_i64(itr)
            self.owner = address(value)

    def onlyOwner(self):
        require_auth(self.owner)

    def transferOwnership(newOwner: address):
        onlyOwner()
        if newOwner != address(0):
            self.owner = newOwner;

class ERC721:
#     Required methods
    def totalSupply(self) -> uint256:
        pass
    def balanceOf(_owner: address) -> uint256:
        pass
    def ownerOf(_tokenId: uint256) -> address:
        pass
    def approve(_to: address, _tokenId: uint256):
        pass
    def transfer(_to: address, _tokenId: uint256):
        pass
    def transferFrom(_from: address, _to: address, _tokenId: uint256):
        pass
    # Events
    def Transfer(_from: address, to: address, tokenId: uint256):
        pass
    def Approval(owner: address, approved: address, tokenId: uint256):
        pass

    # Optional
    # function name() public view returns (string name);
    # function symbol() public view returns (string symbol);
    # function tokensOfOwner(address _owner) external view returns (uint256[] tokenIds);
    # function tokenMetadata(uint256 _tokenId, string _preferredTransport) public view returns (string infoUrl);

    # ERC-165 Compatibility (https://github.com/ethereum/EIPs/issues/165)
#    function supportsInterface(bytes4 _interfaceID) external view returns (bool);
    def supportsInterface(_interfaceID) -> bool:
        pass

#/ @title SEKRETOOOO
class GeneScienceInterface:
    #/ @dev simply a boolean to indicate this is the contract we expect to be
    def isGeneScience() -> bool:
        pass

    #/ @dev given genes of kitten 1 & 2, return a genetic combination - may have a random factor
    #/ @param genes1 genes of mom
    #/ @param genes2 genes of sire
    #/ @return the genes that are supposed to be passed down the child
    def mixGenes(genes1: uint256, genes2: uint256, targetBlock: uint256) ->uint256:
        pass

#/ @title A facet of KittyCore that manages special access privileges.
#/ @author Axiom Zen (https://www.axiomzen.co)
#/ @dev See the KittyCore contract documentation to understand how the various contract facets are arranged.
class KittyAccessControl:
    # This facet controls access control for CryptoKitties. There are four roles managed here:
    #
    #     - The CEO: The CEO can reassign other roles and change the addresses of our dependent smart
    #         contracts. It is also the only role that can unpause the smart contract. It is initially
    #         set to the address that created the smart contract in the KittyCore constructor.
    #
    #     - The CFO: The CFO can withdraw funds from KittyCore and its auction contracts.
    #
    #     - The COO: The COO can release gen0 kitties to auction, and mint promo cats.
    #
    # It should be noted that these roles are distinct without overlap in their access abilities, the
    # abilities listed for each role above are exhaustive. In particular, while the CEO can assign any
    # address to any role, the CEO address itself doesn't have the ability to act in those roles. This
    # restriction is intentional so that we aren't tempted to use the CEO address frequently out of
    # convenience. The less we use an address, the less likely it is that we somehow compromise the
    # account.

    #/ @dev Emited when contract is upgraded - See README.md for updgrade plan
    def ContractUpgrade(newContract: address):
        # The addresses of the accounts (or contracts) that can execute actions within each roles.
        self._ceoAddress = None
        self._cfoAddress = None
        self._cooAddress = None
        # @dev Keeps track whether the contract is paused. When that is true, most actions are blocked
        self._paused = False

    @property
    def ceoAddress(self):
        return self._ceoAddress

    @ceoAddress.setter
    def ceoAddress(self, value):
        self._ceoAddress = value

    @property
    def cfoAddress(self):
        return self._cfoAddress

    @cfoAddress.setter
    def cfoAddress(self, value):
        self._cfoAddress = value

    @property
    def cooAddress(self):
        return self._cooAddress

    @cooAddress.setter
    def cooAddress(self, value):
        self._cooAddress = value

    @property
    def paused(self):
        return self._paused

    @paused.setter
    def paused(self, value):
        self._paused = value

    #/ @dev Access modifier for CEO-only functionality
    def onlyCEO(func):
       def func_wrapper(self, *args):
           require_auth(self._ceoAddress)
           return func(self, *args)
       return func_wrapper

    #/ @dev Access modifier for CFO-only functionality
    def onlyCFO():
       def func_wrapper(self, *args):
           require_auth(self._cfoAddress)
           return func(self, *args)
       return func_wrapper

    #/ @dev Access modifier for COO-only functionality
    def onlyCOO():
       def func_wrapper(self, *args):
           require_auth(self._cooAddress)
           return func(self, *args)
       return func_wrapper

    def onlyCLevel(func):
       def func_wrapper(self, *args):
           eosio_assert(msg.sender == cooAddress or msg.sender == ceoAddress or msg.sender == cfoAddress, "only clevel")
           return func(self, *args)
       return func_wrapper

    #/ @dev Assigns a new address to act as the CEO. Only available to the current CEO.
    #/ @param _newCEO The address of the new CEO
    @onlyCEO
    def setCEO(_newCEO: address):
        require(_newCEO != address(0))
        self.ceoAddress = _newCEO

    #/ @dev Assigns a new address to act as the CFO. Only available to the current CEO.
    #/ @param _newCFO The address of the new CFO
    @onlyCEO
    def setCFO(_newCFO: address):
        require(_newCFO != address(0))
        self.cfoAddress = _newCFO;

    #/ @dev Assigns a new address to act as the COO. Only available to the current CEO.
    #/ @param _newCOO The address of the new COO
    @onlyCEO
    def setCOO(_newCOO: address):
        require(_newCOO != address(0))
        self.cooAddress = _newCOO

    #/*** Pausable functionality adapted from OpenZeppelin ***/

    #/ @dev Modifier to allow actions only when the contract IS NOT paused
    def whenNotPaused(self):
       def func_wrapper(self, *args):
           require(not self._paused)
           return func(self, *args)
       return func_wrapper

    #/ @dev Modifier to allow actions only when the contract IS paused
    def whenPaused(self):
       def func_wrapper(self, *args):
           require(self._paused)
           return func(self, *args)
       return func_wrapper

    #/ @dev Called by any "C-level" role to pause the contract. Used only when
    #/  a bug or exploit is detected and we need to limit damage.
    @onlyCLevel
    def pause(self):
        self.whenNotPaused()
        self.paused = True;

    #/ @dev Unpauses the smart contract. Can only be called by the CEO, since
    #/  one reason we may pause the contract is when CFO or COO accounts are
    #/  compromised.
    #/ @notice This is public rather than external so it can be called by
    #/  derived contracts.
    @onlyCEO
    def unpause():
        self.whenPaused()
        # can't unpause if contract was upgraded
        self.paused = False

#/*** DATA TYPES ***/

# @dev The main Kitty struct. Every cat in CryptoKitties is represented by a copy
#  of this structure, so great care was taken to ensure that it fits neatly into
#  exactly two 256-bit words. Note that the order of the members in this structure
#  is important because of the byte-packing rules used by Ethereum.
#  Ref: http://solidity.readthedocs.io/en/develop/miscellaneous.html

class Kitty:
    def __init__(self, _genes=uint256(0), _birthTime=uint64(0), _cooldownEndBlock = uint64(0),
                 _matronId=uint32(0), _sireId=uint32(0), siringWithId=uint32(0), 
                 _cooldownIndex = uint16(0), _generation = uint16(0)):
        # The Kitty's genetic code is packed into these 256-bits, the format is
        # sooper-sekret! A cat's genes never change.
        #uint256 genes;
        self.genes = _genes

        # The timestamp from the block when this cat came into existence.
        #uint64 birthTime;
        self.birthTime = _birthTime

        # The minimum timestamp after which this cat can engage in breeding
        # activities again. This same timestamp is used for the pregnancy
        # timer (for matrons) as well as the siring cooldown.
        #uint64 cooldownEndBlock;
        self.cooldownEndBlock = _cooldownEndBlock

        # The ID of the parents of this kitty, set to 0 for gen0 cats.
        # Note that using 32-bit unsigned integers limits us to a "mere"
        # 4 billion cats. This number might seem small until you realize
        # that Ethereum currently has a limit of about 500 million
        # transactions per year! So, this definitely won't be a problem
        # for several years (even as Ethereum learns to scale).
        #uint32 matronId;
        self.matronId = _matronId
        #uint32 sireId;
        self.sireId = _sireId

        # Set to the ID of the sire cat for matrons that are pregnant,
        # zero otherwise. A non-zero value here is how we know a cat
        # is pregnant. Used to retrieve the genetic material for the new
        # kitten when the birth transpires.
        #uint32 siringWithId;
        self.siringWithId = _siringWithId

        # Set to the index in the cooldown array (see below) that represents
        # the current cooldown duration for this Kitty. This starts at zero
        # for gen0 cats, and is initialized to floor(generation/2) for others.
        # Incremented by one for each successful breeding action, regardless
        # of whether this cat is acting as matron or sire.
        #uint16 cooldownIndex;
        self.cooldownIndex = _cooldownIndex

        # The "generation number" of this cat. Cats minted by the CK contract
        # for sale are called "gen0" and have a generation number of 0. The
        # generation number of all other cats is the larger of the two generation
        # numbers of their parents, plus one.
        # (i.e. max(matron.generation, sire.generation) + 1)
        #uint16 generation;
        self.generation = _generation
        
# @title Base contract for CryptoKitties. Holds all common structs, events and base variables.
# @author Axiom Zen (https://www.axiomzen.co)
# @dev See the KittyCore contract documentation to understand how the various contract facets are arranged.
class KittyBase : KittyAccessControl:
    #/*** EVENTS ***/
    def __init__(self):
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
    
        # An approximation of currently how many seconds are in between blocks.
        self.secondsPerBlock = uint256(15)
    
        #/*** STORAGE ***/
    
        # @dev An array containing the Kitty struct for all Kitties in existence. The ID
        #  of each cat is actually an index into this array. Note that ID 0 is a negacat,
        #  the unKitty, the mythical beast that is the parent of all gen0 cats. A bizarre
        #  creature that is both matron and sire... to itself! Has an invalid genetic code.
        #  In other words, cat ID 0 is invalid... ;-)
        #Kitty[] kitties;
        self.kitties = List()
    
        # @dev A mapping from cat IDs to the address that owns them. All cats have
        #  some valid owner address, even gen0 cats are created with a non-zero owner.
        #mapping (uint256 => address) public kittyIndexToOwner;
        self.kittyIndexToOwner = List()
    
        # @dev A mapping from owner address to count of tokens that address owns.
        #  Used internally inside balanceOf() to resolve ownership count.
        #mapping (address => uint256) ownershipTokenCount;
        self.ownershipTokenCount = Dict()
    
        # @dev A mapping from KittyIDs to an address that has been approved to call
        #  transferFrom(). Each Kitty can only have one approved address for transfer
        #  at any time. A zero value means no approval is outstanding.
        #mapping (uint256 => address) public kittyIndexToApproved;
        self.kittyIndexToApproved = Dict()
    
        # @dev A mapping from KittyIDs to an address that has been approved to use
        #  this Kitty for siring via breedWith(). Each Kitty can only have one approved
        #  address for siring at any time. A zero value means no approval is outstanding.
        #mapping (uint256 => address) public sireAllowedToAddress;
        self.sireAllowedToAddress = Dict()
    
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
    def Birth(owner: address, kittyId: uint256, matronId: uint256, sireId: uint256, genes: uint256):
        pass

    # @dev Transfer event as defined in current draft of ERC721. Emitted every time a kitten
    #  ownership is assigned, including births.
    #event Transfer(address from, address to, uint256 tokenId);
    def Transfer(_from: address, to: address, tokenId: uint256):
        pass

    #/*** CONSTANTS ***/

    # @dev A lookup table indicating the cooldown duration after any successful
    #  breeding action, called "pregnancy time" for matrons and "siring cooldown"
    #  for sires. Designed such that the cooldown roughly doubles each time a cat
    #  is bred, encouraging owners not to just keep breeding the same cat over
    #  and over again. Caps out at one week (a cat can breed an unbounded number
    #  of times, and the maximum cooldown is always seven days).


    # @dev Assigns ownership of a specific Kitty to an address.
    def _transfer(address _from, address _to, uint256 _tokenId):
        # Since the number of kittens is capped to 2^32 we can't overflow this
        self.ownershipTokenCount[_to]++
        # transfer ownership
        self.kittyIndexToOwner[_tokenId] = _to
        # When creating new kittens _from is 0x0, but we can't account that address.
        if _from != address(0):
            ownershipTokenCount[_from]--
            # once the kitten is transferred also clear sire allowances
            del sireAllowedToAddress[_tokenId]
            # clear any previously approved ownership exchange
            del kittyIndexToApproved[_tokenId]

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
    def _createKitty(_matronId: uint256, _sireId: uint256, _generation: uint256, 
                     _genes: uint256, _owner: address) -> uint
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

        newKittenId = kitties.push(_kitty) - 1

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
        _transfer(0, _owner, newKittenId)
        return newKittenId

    # Any C-level can fix how many seconds per blocks are currently observed.
    def setSecondsPerBlock(secs: uint256):
        self.onlyCLevel()
        require(secs < cooldowns[0])
        self.secondsPerBlock = secs


# @title The external contract that is responsible for generating metadata for the kitties,
#  it has one function that will return the data as bytes.
class ERC721Metadata:
    # @dev Given a token Id, returns a byte array that is supposed to be converted into string.
    def getMetadata(uint256 _tokenId, string) -> (Dict, uint256) {
        buffer = Dict()
        count = 0
        if (_tokenId == 1) {
            buffer[0] = "Hello World! :D";
            count = 15;
        } else if (_tokenId == 2) {
            buffer[0] = "I would definitely choose a medi";
            buffer[1] = "um length string.";
            count = 49;
        } else if (_tokenId == 3) {
            buffer[0] = "Lorem ipsum dolor sit amet, mi e";
            buffer[1] = "st accumsan dapibus augue lorem,";
            buffer[2] = " tristique vestibulum id, libero";
            buffer[3] = " suscipit varius sapien aliquam.";
            count = 128;
        return (buffer, uint256(count))


# @title The facet of the CryptoKitties core contract that manages ownership, ERC-721 (draft) compliant.
# @author Axiom Zen (https://www.axiomzen.co)
# @dev Ref: https://github.com/ethereum/EIPs/issues/721
#  See the KittyCore contract documentation to understand how the various contract facets are arranged.
class KittyOwnership(KittyBase, ERC721):
    def __init__(self):
        KittyBase.__init__(self)
        ERC721.__init__(self)
        # @notice Name and symbol of the non fungible token, as defined in ERC721.
        #string public constant name = "CryptoKitties";
        #string public constant symbol = "CK";

        self.name = "CryptoKitties"
        self.symbol = "CK"

        # The contract that will return kitty metadata
        #ERC721Metadata public erc721Metadata;
        self.erc721Metadata = ERC721Metadata()

        self.InterfaceSignature_ERC165 = bytes4(keccak256('supportsInterface(bytes4)'));

        self.InterfaceSignature_ERC721 = bytes4(keccak256('InterfaceSignature_ERC721(bytes4)'));

        '''TODO
        bytes4 constant InterfaceSignature_ERC721 =
            bytes4(keccak256('name()')) ^
            bytes4(keccak256('symbol()')) ^
            bytes4(keccak256('totalSupply()')) ^
            bytes4(keccak256('balanceOf(address)')) ^
            bytes4(keccak256('ownerOf(uint256)')) ^
            bytes4(keccak256('approve(address,uint256)')) ^
            bytes4(keccak256('transfer(address,uint256)')) ^
            bytes4(keccak256('transferFrom(address,address,uint256)')) ^
            bytes4(keccak256('tokensOfOwner(address)')) ^
            bytes4(keccak256('tokenMetadata(uint256,string)'));
        '''
    # @notice Introspection interface as per ERC-165 (https://github.com/ethereum/EIPs/issues/165).
    #  Returns true for any standardized interfaces implemented by this contract. We implement
    #  ERC-165 (obviously!) and ERC-721.
    def supportsInterface(bytes4 _interfaceID) -> bool:
        # DEBUG ONLY
        #require((InterfaceSignature_ERC165 == 0x01ffc9a7) && (InterfaceSignature_ERC721 == 0x9a20483d));
        return (_interfaceID == InterfaceSignature_ERC165) or (_interfaceID == InterfaceSignature_ERC721)


    # @dev Set the address of the sibling contract that tracks metadata.
    #  CEO only.
    @onlyCEO
    def setMetadataAddress(_contractAddress: address):
        self.erc721Metadata = ERC721Metadata(_contractAddress)

    # Internal utility functions: These functions all assume that their input arguments
    # are valid. We leave it to public methods to sanitize their inputs and follow
    # the required logic.

    # @dev Checks if a given address is the current owner of a particular Kitty.
    # @param _claimant the address we are validating against.
    # @param _tokenId kitten id, only valid when > 0
    def _owns(_claimant: address, _tokenId: uint256) -> bool:
        return kittyIndexToOwner[_tokenId] == _claimant

    # @dev Checks if a given address currently has transferApproval for a particular Kitty.
    # @param _claimant the address we are confirming kitten is approved for.
    # @param _tokenId kitten id, only valid when > 0
    def _approvedFor(_claimant: address, _tokenId: uint256) -> bool:
        return kittyIndexToApproved[_tokenId] == _claimant

    # @dev Marks an address as being approved for transferFrom(), overwriting any previous
    #  approval. Setting _approved to address(0) clears all transfer approval.
    #  NOTE: _approve() does NOT send the Approval event. This is intentional because
    #  _approve() and transferFrom() are used together for putting Kitties on auction, and
    #  there is no value in spamming the log with Approval events in that case.
    def _approve(_tokenId: uint256, _approved: address):
        kittyIndexToApproved[_tokenId] = _approved

    # @notice Returns the number of Kitties owned by a specific address.
    # @param _owner The owner address to check.
    # @dev Required for ERC-721 compliance
    def balanceOf(_owner: address)  -> uint256:
        return ownershipTokenCount[_owner]
    

    # @notice Transfers a Kitty to another address. If transferring to a smart
    #  contract be VERY CAREFUL to ensure that it is aware of ERC-721 (or
    #  CryptoKitties specifically) or your Kitty may be lost forever. Seriously.
    # @param _to The address of the recipient, can be a user or contract.
    # @param _tokenId The ID of the Kitty to transfer.
    # @dev Required for ERC-721 compliance.
    def transfer(_to: address, _tokenId: uint256):
        self.whenNotPaused()
        # Safety check to prevent against an unexpected 0x0 default.
        require(_to != address(0))
        # Disallow transfers to this contract to prevent accidental misuse.
        # The contract should never own any kitties (except very briefly
        # after a gen0 cat is created and before it goes on auction).
        require(_to != address(this))
        # Disallow transfers to the auction contracts to prevent accidental
        # misuse. Auction contracts should only take ownership of kitties
        # through the allow + transferFrom flow.
        require(_to != address(saleAuction))
        require(_to != address(siringAuction))
        # You can only send your own cat.
        require(_owns(msg.sender, _tokenId))
        # Reassign ownership, clear pending approvals, emit Transfer event.
        _transfer(msg.sender, _to, _tokenId)

    # @notice Grant another address the right to transfer a specific Kitty via
    #  transferFrom(). This is the preferred flow for transfering NFTs to contracts.
    # @param _to The address to be granted transfer approval. Pass address(0) to
    #  clear all approvals.
    # @param _tokenId The ID of the Kitty that can be transferred if this call succeeds.
    # @dev Required for ERC-721 compliance.
    def approve(_to: address, _tokenId: uint256):
        self.whenNotPaused()
        # Only an owner can grant transfer approval.
        require(self._owns(msg.sender, _tokenId))

        # Register the approval (replacing any previous approval).
        self._approve(_tokenId, _to)

        # Emit approval event.
        self.Approval(msg.sender, _to, _tokenId)

    # @notice Transfer a Kitty owned by another address, for which the calling address
    #  has previously been granted transfer approval by the owner.
    # @param _from The address that owns the Kitty to be transfered.
    # @param _to The address that should take ownership of the Kitty. Can be any address,
    #  including the caller.
    # @param _tokenId The ID of the Kitty to be transferred.
    # @dev Required for ERC-721 compliance.
    def transferFrom(_from: address, _to: address, _tokenId: uint256):
        self.whenNotPaused()
        # Safety check to prevent against an unexpected 0x0 default.
        require(_to != address(0))
        # Disallow transfers to this contract to prevent accidental misuse.
        # The contract should never own any kitties (except very briefly
        # after a gen0 cat is created and before it goes on auction).
        require(_to != address(this))
        # Check for approval and valid ownership
        require(self._approvedFor(msg.sender, _tokenId))
        require(self._owns(_from, _tokenId))

        # Reassign ownership (also clears pending approvals and emits Transfer event).
        self._transfer(_from, _to, _tokenId)

    # @notice Returns the total number of Kitties currently in existence.
    # @dev Required for ERC-721 compliance.
    def totalSupply() -> uint:
        return kitties.length - 1

    # @notice Returns the address currently assigned ownership of a given Kitty.
    # @dev Required for ERC-721 compliance.
    def ownerOf(_tokenId: uint256) -> address:
        owner = kittyIndexToOwner[_tokenId]
        require(owner != address(0))
        return owner

    # @notice Returns a list of all Kitty IDs assigned to an address.
    # @param _owner The owner whose Kitties we are interested in.
    # @dev This method MUST NEVER be called by smart contract code. First, it's fairly
    #  expensive (it walks the entire Kitty array looking for cats belonging to owner),
    #  but it also returns a dynamic array, which is only supported for web3 calls, and
    #  not contract-to-contract calls.
    def tokensOfOwner(address _owner) -> List(uint256):
        uint256 tokenCount = self.balanceOf(_owner)

        result = List([],uint256)

        if tokenCount == 0:
            # Return an empty array
            return result
        else:
            #FIXME memory type
#            uint256[] memory result = new uint256[](tokenCount);
#            uint256 totalCats = self.totalSupply();
#            uint256 resultIndex = 0;
            result = List(size = tokenCount, value_type=uint256)
            totalCats = self.totalSupply();]
            resultIndex = 0

            # We count on the fact that all cats have IDs starting at 1 and increasing
            # sequentially up to the totalCat count.
#            uint256 catId;

            for catId in range(1, totalCats+1):
                if self.kittyIndexToOwner[catId] == _owner:
                    result[resultIndex] = catId
                    resultIndex++

            return result

    # @dev Adapted from memcpy() by @arachnid (Nick Johnson <arachnid@notdot.net>)
    #  This method is licenced under the Apache License.
    #  Ref: https://github.com/Arachnid/solidity-stringutils/blob/2f6ca9accb48ae14c66f1437ec50ed19a0616f78/strings.sol

    def _memcpy(_dest: uint, _src: uint, _len: uint):
        pass
    '''
    def _memcpy(uint _dest, uint _src, uint _len) private view {
        # Copy word-length chunks while possible
        for(; _len >= 32; _len -= 32) {
            assembly {
                mstore(_dest, mload(_src))
            }
            _dest += 32;
            _src += 32;
        }

        # Copy remaining bytes
        uint256 mask = 256 ** (32 - _len) - 1;
        assembly {
            let srcpart := and(mload(_src), not(mask))
            let destpart := and(mload(_dest), mask)
            mstore(_dest, or(destpart, srcpart))
        }
    }
    '''
    
    # @dev Adapted from toString(slice) by @arachnid (Nick Johnson <arachnid@notdot.net>)
    #  This method is licenced under the Apache License.
    #  Ref: https://github.com/Arachnid/solidity-stringutils/blob/2f6ca9accb48ae14c66f1437ec50ed19a0616f78/strings.sol
    #FIXME
    def _toString(_rawBytes, _stringLength) -> str：
        assert False

    '''
    def _toString(bytes32[4] _rawBytes, uint256 _stringLength) private view returns (string) {
        var outputString = new string(_stringLength);
        uint256 outputPtr;
        uint256 bytesPtr;

        assembly {
            outputPtr := add(outputString, 32)
            bytesPtr := _rawBytes
        }

        _memcpy(outputPtr, bytesPtr, _stringLength);

        return outputString;
    '''

    # @notice Returns a URI pointing to a metadata package for this token conforming to
    #  ERC-721 (https://github.com/ethereum/EIPs/issues/721)
    # @param _tokenId The ID number of the Kitty whose metadata should be returned.
    def tokenMetadata(_tokenId: uint256, _preferredTransport: str) -> string:
        require(erc721Metadata != address(0))
#        bytes32[4] memory buffer;
#        uint256 count;
        buffer, count = self.erc721Metadata.getMetadata(_tokenId, _preferredTransport)
        return self._toString(buffer, count)



# @title A facet of KittyCore that manages Kitty siring, gestation, and birth.
# @author Axiom Zen (https://www.axiomzen.co)
# @dev See the KittyCore contract documentation to understand how the various contract facets are arranged.
class KittyBreeding(KittyOwnership):
    def __init__(self):
        # @notice The minimum payment required to use breedWithAuto(). This fee goes towards
        #  the gas cost paid by whatever calls giveBirth(), and can be dynamically updated by
        #  the COO role as the gas price changes.
        #uint256 public autoBirthFee = 2 finney;
        self.autoBirthFee = uint256(2)
    
        # Keeps track of number of pregnant kitties.
        #uint256 public pregnantKitties;
        self.pregnantKitties = uint256(0)
    
        # @dev The address of the sibling contract that is used to implement the sooper-sekret
        #  genetic combination algorithm.
        #GeneScienceInterface public geneScience;
        self.geneScience = GeneScienceInterface()

    # @dev The Pregnant event is fired when two cats successfully breed and the pregnancy
    #  timer begins for the matron.
    #FIXME
    #event Pregnant(address owner, uint256 matronId, uint256 sireId, uint256 cooldownEndBlock);
    def Pregnant(owner: address, matronId: uint256, sireId: uint256, cooldownEndBlock: uint256):
        raise Exception('Fix event')

    # @dev Update the address of the genetic contract, can only be called by the CEO.
    # @param _address An address of a GeneScience contract instance to be used from this point forward.
    @onlyCEO
    def setGeneScienceAddress(address _address):
        candidateContract = GeneScienceInterface(_address)

        # NOTE: verify that a contract is what we expect - https://github.com/Lunyr/crowdsale-contracts/blob/cfadd15986c30521d8ba7d5b6f57b4fefcc7ac38/contracts/LunyrToken.sol#L117
        require(candidateContract.isGeneScience());

        # Set the new contract address
        geneScience = candidateContract;

    # @dev Checks that a given kitten is able to breed. Requires that the
    #  current cooldown is finished (for sires) and also checks that there is
    #  no pending pregnancy.
    def _isReadyToBreed(_kit: Kitty) -> bool:
        # In addition to checking the cooldownEndBlock, we also need to check to see if
        # the cat has a pending birth; there can be some period of time between the end
        # of the pregnacy timer and the birth event.
        return (_kit.siringWithId == 0) and (_kit.cooldownEndBlock <= uint64(block.number));


    # @dev Check if a sire has authorized breeding with this matron. True if both sire
    #  and matron have the same owner, or if the sire has given siring permission to
    #  the matron's owner (via approveSiring()).
    def _isSiringPermitted(_sireId: uint256, _matronId: uint256) -> bool:
        matronOwner = self.kittyIndexToOwner[_matronId]
        sireOwner = self.kittyIndexToOwner[_sireId]

        # Siring is okay if they have same owner, or if the matron's owner was given
        # permission to breed with this sire.
        return matronOwner == sireOwner or self.sireAllowedToAddress[_sireId] == matronOwner

    # @dev Set the cooldownEndTime for the given Kitty, based on its current cooldownIndex.
    #  Also increments the cooldownIndex (unless it has hit the cap).
    # @param _kitten A reference to the Kitty in storage which needs its timer started.
    #def _triggerCooldown(Kitty storage _kitten) internal {

    def _triggerCooldown(_kitten: Kitty):
        # Compute an estimation of the cooldown time in blocks (based on current cooldownIndex).
        _kitten.cooldownEndBlock = uint64((self.cooldowns[_kitten.cooldownIndex]/self.secondsPerBlock) + block.number);

        # Increment the breeding count, clamping it at 13, which is the length of the
        # cooldowns array. We could check the array size dynamically, but hard-coding
        # this as a constant saves gas. Yay, Solidity!
        if _kitten.cooldownIndex < 13:
            _kitten.cooldownIndex += 1

    # @notice Grants approval to another user to sire with one of your Kitties.
    # @param _addr The address that will be able to sire with your Kitty. Set to
    #  address(0) to clear all siring approvals for this Kitty.
    # @param _sireId A Kitty that you own that _addr will now be able to sire with.
    def approveSiring(_addr: address, _sireId: uint256):
        self.whenNotPaused()
        require(_owns(msg.sender, _sireId))
        sireAllowedToAddress[_sireId] = _addr

    # @dev Updates the minimum payment required for calling giveBirthAuto(). Can only
    #  be called by the COO address. (This fee is used to offset the gas cost incurred
    #  by the autobirth daemon).
    def setAutoBirthFee(val: uint256):
        self.onlyCOO()
        self.autoBirthFee = val;

    # @dev Checks to see if a given Kitty is pregnant and (if so) if the gestation
    #  period has passed.
    def _isReadyToGiveBirth(_matron: Kitty) -> bool:
        return (_matron.siringWithId != 0) and (_matron.cooldownEndBlock <= uint64(block.number))

    # @notice Checks that a given kitten is able to breed (i.e. it is not pregnant or
    #  in the middle of a siring cooldown).
    # @param _kittyId reference the id of the kitten, any user can inquire about it
    def isReadyToBreed(_kittyId: uint256) -> bool:
        require(_kittyId > 0)
#        Kitty storage kit = kitties[_kittyId];
        kit = self.kitties[_kittyId]
        return self._isReadyToBreed(kit)

    # @dev Checks whether a kitty is currently pregnant.
    # @param _kittyId reference the id of the kitten, any user can inquire about it
    def isPregnant(_kittyId: uint256) -> bool:
        require(_kittyId > 0);
        # A kitty is pregnant if and only if this field is set
        return kitties[_kittyId].siringWithId != 0;

    # @dev Internal check to see if a given sire and matron are a valid mating pair. DOES NOT
    #  check ownership permissions (that is up to the caller).
    # @param _matron A reference to the Kitty struct of the potential matron.
    # @param _matronId The matron's ID.
    # @param _sire A reference to the Kitty struct of the potential sire.
    # @param _sireId The sire's ID
    def _isValidMatingPair(_matron: Kitty, _matronId: uint256, _sire: Kitty, _sireId: uint256) -> bool:
#        Kitty storage _matron,
      # A Kitty can't breed with itself!
        if _matronId == _sireId:
            return False

        # Kitties can't breed with their parents.
        if _matron.matronId == _sireId or _matron.sireId == _sireId:
            return False

        if _sire.matronId == _matronId or _sire.sireId == _matronId:
            return False

        # We can short circuit the sibling check (below) if either cat is
        # gen zero (has a matron ID of zero).
        if _sire.matronId == 0 or _matron.matronId == 0:
            return True

        # Kitties can't breed with full or half siblings.
        if _sire.matronId == _matron.matronId or _sire.matronId == _matron.sireId:
            return False

        if _sire.sireId == _matron.matronId or _sire.sireId == _matron.sireId:
            return False

        # Everything seems cool! Let's get DTF.
        return True

    # @dev Internal check to see if a given sire and matron are a valid mating pair for
    #  breeding via auction (i.e. skips ownership and siring approval checks).
    def _canBreedWithViaAuction(_matronId: uint256, _sireId: uint256) -> bool:
        matron = self.kitties[_matronId]
        sire = self.kitties[_sireId]
        return self._isValidMatingPair(matron, _matronId, sire, _sireId)

    # @notice Checks to see if two cats can breed together, including checks for
    #  ownership and siring approvals. Does NOT check that both cats are ready for
    #  breeding (i.e. breedWith could still fail until the cooldowns are finished).
    #  TODO: Shouldn't this check pregnancy and cooldowns?!?
    # @param _matronId The ID of the proposed matron.
    # @param _sireId The ID of the proposed sire.
    def canBreedWith(_matronId: uint256, _sireId: uint256) -> bool :
        require(_matronId > 0)
        require(_sireId > 0)
        matron = self.kitties[_matronId]
        sire = self.kitties[_sireId];
        return self._isValidMatingPair(matron, _matronId, sire, _sireId) and self._isSiringPermitted(_sireId, _matronId)

    # @dev Internal utility function to initiate breeding, assumes that all breeding
    #  requirements have been checked.
    def _breedWith(_matronId: uint256, _sireId: uint256):
        # Grab a reference to the Kitties from storage.
        sire = self.kitties[_sireId]
        matron = self.kitties[_matronId]

        # Mark the matron as pregnant, keeping track of who the sire is.
        matron.siringWithId = uint32(_sireId)

        # Trigger the cooldown for both parents.
        self._triggerCooldown(sire)
        self._triggerCooldown(matron)

        # Clear siring permission for both parents. This may not be strictly necessary
        # but it's likely to avoid confusion!
        del sireAllowedToAddress[_matronId]
        del sireAllowedToAddress[_sireId]

        # Every time a kitty gets pregnant, counter is incremented.
        self.pregnantKitties += 1

        # Emit the pregnancy event.
        self.Pregnant(kittyIndexToOwner[_matronId], _matronId, _sireId, matron.cooldownEndBlock)

    # @notice Breed a Kitty you own (as matron) with a sire that you own, or for which you
    #  have previously been given Siring approval. Will either make your cat pregnant, or will
    #  fail entirely. Requires a pre-payment of the fee given out to the first caller of giveBirth()
    # @param _matronId The ID of the Kitty acting as matron (will end up pregnant if successful)
    # @param _sireId The ID of the Kitty acting as sire (will begin its siring cooldown if successful)
    def breedWithAuto(_matronId: uint256, _sireId: uint256):
        self.whenNotPaused()
        # Checks for payment.
        require(msg.value >= autoBirthFee);

        # Caller must own the matron.
        require(_owns(msg.sender, _matronId));

        # Neither sire nor matron are allowed to be on auction during a normal
        # breeding operation, but we don't need to check that explicitly.
        # For matron: The caller of this function can't be the owner of the matron
        #   because the owner of a Kitty on auction is the auction house, and the
        #   auction house will never call breedWith().
        # For sire: Similarly, a sire on auction will be owned by the auction house
        #   and the act of transferring ownership will have cleared any oustanding
        #   siring approval.
        # Thus we don't need to spend gas explicitly checking to see if either cat
        # is on auction.

        # Check that matron and sire are both owned by caller, or that the sire
        # has given siring permission to caller (i.e. matron's owner).
        # Will fail for _sireId = 0
        require(self._isSiringPermitted(_sireId, _matronId));

        # Grab a reference to the potential matron
        matron = self.kitties[_matronId];

        # Make sure matron isn't pregnant, or in the middle of a siring cooldown
        require(self._isReadyToBreed(matron));

        # Grab a reference to the potential sire
        sire = self.kitties[_sireId];

        # Make sure sire isn't pregnant, or in the middle of a siring cooldown
        require(self._isReadyToBreed(sire));

        # Test that these cats are a valid mating pair.
        require(self._isValidMatingPair(matron, _matronId, sire,_sireId));

        # All checks passed, kitty gets pregnant!
        self._breedWith(_matronId, _sireId);

    # @notice Have a pregnant Kitty give birth!
    # @param _matronId A Kitty ready to give birth.
    # @return The Kitty ID of the new kitten.
    # @dev Looks at a given Kitty and, if pregnant and if the gestation period has passed,
    #  combines the genes of the two parents to create a new kitten. The new Kitty is assigned
    #  to the current owner of the matron. Upon successful completion, both the matron and the
    #  new kitten will be ready to breed again. Note that anyone can call this function (if they
    #  are willing to pay the gas!), but the new kitten always goes to the mother's owner.
    def giveBirth(_matronId: uint256) -> uint256
        self.whenNotPaused()
        # Grab a reference to the matron in storage.
        matron = self.kitties[_matronId];

        # Check that the matron is a valid cat.
        require(matron.birthTime != 0);

        # Check that the matron is pregnant, and that its time has come!
        require(self._isReadyToGiveBirth(matron));

        # Grab a reference to the sire in storage.
        sireId = matron.siringWithId;
        sire = self.kitties[sireId];

        # Determine the higher generation number of the two parents
        parentGen = matron.generation;
        if sire.generation > matron.generation:
            parentGen = sire.generation

        # Call the sooper-sekret gene mixing operation.
        childGenes = self.geneScience.mixGenes(matron.genes, sire.genes, matron.cooldownEndBlock - 1);

        # Make the new kitten!
        owner = kittyIndexToOwner[_matronId]
        kittenId = self._createKitty(_matronId, matron.siringWithId, parentGen + 1, childGenes, owner)

        # Clear the reference to sire from the matron (REQUIRED! Having siringWithId
        # set is what marks a matron as being pregnant.)
        del matron.siringWithId;

        # Every time a kitty gives birth counter is decremented.
        self.pregnantKitties--;

        # Send the balance fee to the person who made birth happen.
        msg.sender.send(autoBirthFee);

        # return the new kitten's ID
        return kittenId;

#Represents an auction on an NFT
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
    self.tokenIdToAuction = Dict(uint256, Auction)

    '''FIXME
    event AuctionCreated(uint256 tokenId, uint256 startingPrice, uint256 endingPrice, uint256 duration);
    event AuctionSuccessful(uint256 tokenId, uint256 totalPrice, address winner);
    event AuctionCancelled(uint256 tokenId);
    '''

    # @dev Returns true if the claimant owns the token.
    # @param _claimant - Address claiming to own the token.
    # @param _tokenId - ID of token whose ownership to verify.
    def _owns(_claimant: address, _tokenId: uint256) -> bool:
        return self.nonFungibleContract.ownerOf(_tokenId) == _claimant

    # @dev Escrows the NFT, assigning ownership to this contract.
    # Throws if the escrow fails.
    # @param _owner - Current owner address of token to escrow.
    # @param _tokenId - ID of token whose approval to verify.
    def _escrow(address _owner, uint256 _tokenId):
        #it will throw if transfer fails
        #FIXME this
        self.nonFungibleContract.transferFrom(_owner, this, _tokenId);

    # @dev Transfers an NFT owned by this contract to another address.
    # Returns true if the transfer succeeds.
    # @param _receiver - Address to transfer NFT to.
    # @param _tokenId - ID of token to transfer.
    def _transfer(_receiver: address, _tokenId: uint256):
        #it will throw if transfer fails
        self.nonFungibleContract.transfer(_receiver, _tokenId)

    # @dev Adds an auction to the list of open auctions. Also fires the
    #  AuctionCreated event.
    # @param _tokenId The ID of the token to be put on auction.
    # @param _auction Auction to add.
    def _addAuction(_tokenId: uint256, _auction: Auction):
        #Require that all auctions have a duration of
        #at least one minute. (Keeps our math from getting hairy!)
        require(_auction.duration >= 1 minutes)
        self.tokenIdToAuction[_tokenId] = _auction;

        self.AuctionCreated(
            uint256(_tokenId),
            uint256(_auction.startingPrice),
            uint256(_auction.endingPrice),
            uint256(_auction.duration)
        )

    # @dev Cancels an auction unconditionally.
    def _cancelAuction(_tokenId: uint256, _seller: address):
        self._removeAuction(_tokenId)
        self._transfer(_seller, _tokenId)
        self.AuctionCancelled(_tokenId)

    # @dev Computes the price and transfers winnings.
    # Does NOT transfer ownership of token.
    def _bid(_tokenId: uint256, _bidAmount: uint256) -> uint256:
        #Get a reference to the auction struct
        auction = self.tokenIdToAuction[_tokenId];

        #Explicitly check that this auction is currently live.
        #(Because of how Ethereum mappings work, we can't just count
        #on the lookup above failing. An invalid _tokenId will just
        #return an auction object that is all zeros.)
        require(self._isOnAuction(auction));

        #Check that the bid is greater than or equal to the current price
        uint256 price = self._currentPrice(auction);
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
    def _removeAuction(_tokenId: uint256):
        del tokenIdToAuction[_tokenId]

    # @dev Returns true if the NFT is on auction.
    # @param _auction - Auction to check.
    def _isOnAuction(_auction: Auction) -> bool:
        return _auction.startedAt > 0

    # @dev Returns current price of an NFT on auction. Broken into two
    #  functions (this one, that computes the duration from the auction
    #  structure, and the other that does the price computation) so we
    #  can easily test that the price computation works correctly.
    def _currentPrice(_auction: Auction) -> uint256:
        uint256 secondsPassed = 0;
        #A bit of insurance against negative values (or wraparound).
        #Probably not necessary (since Ethereum guarnatees that the
        #now variable doesn't ever go backwards).
        if now > _auction.startedAt:
            secondsPassed = now - _auction.startedAt;

        return self._computeCurrentPrice(
            _auction.startingPrice,
            _auction.endingPrice,
            _auction.duration,
            secondsPassed
        )

    # @dev Computes the current price of an auction. Factored out
    #  from _currentPrice so we can run extensive unit tests.
    #  When testing, make this function public and turn on
    #  `Current price computation` test suite.
    def _computeCurrentPrice(_startingPrice: uint256, _endingPrice: uint256, _duration: uint256, _secondsPassed: uint256) -> uint256:
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
            totalPriceChange = int256(_endingPrice) - int256(_startingPrice);

            #This multiplication can't overflow, _secondsPassed will easily fit within
            #64-bits, and totalPriceChange will easily fit within 128-bits, their product
            #will always fit within 256-bits.
            currentPriceChange = totalPriceChange * int256(_secondsPassed) / int256(_duration);

            #currentPriceChange can be negative, but if so, will have a magnitude
            #less that _startingPrice. Thus, this result will always end up positive.
            currentPrice = int256(_startingPrice) + currentPriceChange;

            return uint256(currentPrice);

    # @dev Computes owner's cut of a sale.
    # @param _price - Sale price of NFT.
    def _computeCut(_price: uint256)  -> uint256:
        #NOTE: We don't use SafeMath (or similar) in this function because
        # all of our entry functions carefully cap the maximum values for
        # currency (at 128-bits), and ownerCut <= 10000 (see the require()
        # statement in the ClockAuction constructor). The result of this
        # function is always guaranteed to be <= _price.
        return _price * ownerCut / 10000;

'''
 * @title Pausable
 * @dev Base contract which allows children to implement an emergency stop mechanism.
'''
 
class Pausable(Ownable):
    '''FIXME: event
      event Pause();
      event Unpause();
    '''

    def __init__(self):
        super(Pausable,self).__init__()
        self.paused = False


    '''
    * @dev modifier to allow actions only when the contract IS paused
    FIXME: modifier
    modifier whenNotPaused() {
      require(!paused);
      _;
    }
    '''
    def whenNotPaused(self):
        require(not self.paused)
    '''
   * @dev modifier to allow actions only when the contract IS NOT paused
    '''
    def whenPaused(self):
        require(self.paused)

   '''
   * @dev called by the owner to pause, triggers stopped state
    '''
    def pause() -> bool:
        self.onlyOwner()
        self.whenNotPaused()
        self.paused = true;
        self.Pause();
        return True;

    '''
   * @dev called by the owner to unpause, returns to normal state
    '''
    def unpause() -> bool:
        self.onlyOwner()
        self.whenPaused()
        self.paused = False
        self.Unpause()
        return True


# @title Clock auction for non-fungible tokens.
# @notice We omit a fallback function to prevent accidental sends to this contract.
class ClockAuction(Pausable, ClockAuctionBase)
    # @dev The ERC-165 interface signature for ERC-721.
    #  Ref: https://github.com/ethereum/EIPs/issues/165
    #  Ref: https://github.com/ethereum/EIPs/issues/721
    bytes4 constant InterfaceSignature_ERC721 = bytes4(0x9a20483d);

    def __init__(self):
        Pausable.__init__(self)
        ClockAuctionBase.__init__(self)

    # @dev Constructor creates a reference to the NFT ownership contract
    #  and verifies the owner cut is in the valid range.
    # @param _nftAddress - address of a deployed contract implementing
    #  the Nonfungible Interface.
    # @param _cut - percent cut the owner takes on each auction, must be
    #  between 0-10,000.
    def ClockAuction(_nftAddress: address, _cut: uint256):
        require(_cut <= 10000)
        self.ownerCut = _cut;

        candidateContract = ERC721(_nftAddress)
        require(candidateContract.supportsInterface(InterfaceSignature_ERC721))
        self.nonFungibleContract = candidateContract
    

    # @dev Remove all Ether from the contract, which is the owner's cuts
    #  as well as any Ether sent directly to the contract address.
    #  Always transfers to the NFT contract, but can be called either by
    #  the owner or the NFT contract.
    def withdrawBalance():
        nftAddress = address(nonFungibleContract);
        require(msg.sender == owner or msg.sender == nftAddress)
        # We are using this boolean method to make sure that even if one fails it will still work
        res = nftAddress.send(this.balance);

    # @dev Creates and begins a new auction.
    # @param _tokenId - ID of token to auction, sender must be owner.
    # @param _startingPrice - Price of item (in wei) at beginning of auction.
    # @param _endingPrice - Price of item (in wei) at end of auction.
    # @param _duration - Length of time to move between starting
    #  price and ending price (in seconds).
    # @param _seller - Seller, if not the message sender
    function createAuction(_tokenId: uint256, _startingPrice: uint256,
                           _endingPrice: uint256,
                            _duration: uint256,
                            _seller: uint256):
        self.whenNotPaused()
        # Sanity check that no inputs overflow how many bits we've allocated
        # to store them in the auction struct.
        require(_startingPrice == uint256(uint128(_startingPrice)))
        require(_endingPrice == uint256(uint128(_endingPrice)))
        require(_duration == uint256(uint64(_duration)))

        require(_owns(msg.sender, _tokenId))
        self._escrow(msg.sender, _tokenId)
        auction = Auction(_seller, uint128(_startingPrice), uint128(_endingPrice), 
                          uint64(_duration),
                          uint64(now()))
        self._addAuction(_tokenId, auction)

    # @dev Bids on an open auction, completing the auction and transferring
    #  ownership of the NFT if enough Ether is supplied.
    # @param _tokenId - ID of token to bid on.
    def bid(_tokenId: uint256)
        self.whenNotPaused()
        # _bid will throw if the bid or funds transfer fails
        self._bid(_tokenId, msg.value);
        self._transfer(msg.sender, _tokenId);

    # @dev Cancels an auction that hasn't been won yet.
    #  Returns the NFT to original owner.
    # @notice This is a state-modifying function that can
    #  be called while the contract is paused.
    # @param _tokenId - ID of token on auction
    def cancelAuction(_tokenId: uint256)
        auction = self.tokenIdToAuction[_tokenId]
        require(self._isOnAuction(auction))
        
        address seller = auction.seller
        require(msg.sender == seller)
        self._cancelAuction(_tokenId, seller)

    # @dev Cancels an auction when the contract is paused.
    #  Only the owner may do this, and NFTs are returned to
    #  the seller. This should only be used in emergencies.
    # @param _tokenId - ID of the NFT on auction to cancel.
    def cancelAuctionWhenPaused(_tokenId: uint256):
        self.whenPaused()
        self.onlyOwner()
        auction = tokenIdToAuction[_tokenId]
        require(_isOnAuction(auction))
        self._cancelAuction(_tokenId, auction.seller)

    # @dev Returns auction info for an NFT on auction.
    # @param _tokenId - ID of NFT on auction.
    def getAuction(_tokenId: uint256):
        auction = self.tokenIdToAuction[_tokenId]
        require(self._isOnAuction(auction))
        return (
            auction.seller,
            auction.startingPrice,
            auction.endingPrice,
            auction.duration,
            auction.startedAt
        )

    # @dev Returns the current price of an auction.
    # @param _tokenId - ID of the token price we are checking.
    def getCurrentPrice(_tokenId: uint256) -> uint256:
        auction = self.tokenIdToAuction[_tokenId]
        require(self._isOnAuction(auction));
        return self._currentPrice(auction);


# @title Reverse auction modified for siring
# @notice We omit a fallback function to prevent accidental sends to this contract.
class SiringClockAuction(ClockAuction):
    # Delegate constructor
    def __init__(_nftAddr: address, _cut: uint256):
        # @dev Sanity check that allows us to ensure that we are pointing to the
        #  right auction in our setSiringAuctionAddress() call.
        self.isSiringClockAuction = True
        ClockAuction.__init__(self, _nftAddr, _cut)

    # @dev Creates and begins a new auction. Since this function is wrapped,
    # require sender to be KittyCore contract.
    # @param _tokenId - ID of token to auction, sender must be owner.
    # @param _startingPrice - Price of item (in wei) at beginning of auction.
    # @param _endingPrice - Price of item (in wei) at end of auction.
    # @param _duration - Length of auction (in seconds).
    # @param _seller - Seller, if not the message sender
    def createAuction(_tokenId: uint256, _startingPrice: uint256,
        _endingPrice: uint256, _duration: uint256, _seller: address):
        # Sanity check that no inputs overflow how many bits we've allocated
        # to store them in the auction struct.
        require(_startingPrice == uint256(uint128(_startingPrice)))
        require(_endingPrice == uint256(uint128(_endingPrice)))
        require(_duration == uint256(uint64(_duration)))

        require(msg.sender == address(nonFungibleContract))
        self._escrow(_seller, _tokenId)
        auction = Auction(
            _seller,
            uint128(_startingPrice),
            uint128(_endingPrice),
            uint64(_duration),
            uint64(now)
        )
        self._addAuction(_tokenId, auction)

    # @dev Places a bid for siring. Requires the sender
    # is the KittyCore contract because all bid methods
    # should be wrapped. Also returns the kitty to the
    # seller rather than the winner.
    def bid(_tokenId: uint256):
        require(msg.sender == address(self.nonFungibleContract))
        seller = self.tokenIdToAuction[_tokenId].seller
        # _bid checks that token ID is valid and will throw if bid fails
        self._bid(_tokenId, msg.value)
        # We transfer the kitty back to the seller, the winner will get
        # the offspring
        self._transfer(seller, _tokenId)

# @title Clock auction modified for sale of kitties
# @notice We omit a fallback function to prevent accidental sends to this contract.
class SaleClockAuction(ClockAuction):
    # Delegate constructor
     def __init__(_nftAddr: address, _cut: uint256):
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
    def createAuction(
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
    def bid(_tokenId: uint256):
        # _bid verifies token ID size
        seller = self.tokenIdToAuction[_tokenId].seller
        price = self._bid(_tokenId, msg.value)
        self._transfer(msg.sender, _tokenId)

        # If not a gen0 auction, exit
        if seller == address(self.nonFungibleContract):
            # Track gen0 sale prices
            self.lastGen0SalePrices[self.gen0SaleCount % 5] = price;
            self.gen0SaleCount++;

    def averageGen0SalePrice() -> uint256:
        sum = uint256(0)
        for i in range(5): 
            sum += lastGen0SalePrices[i];
        return uint256(sum / 5)

# @title Handles creating auctions for sale and siring of kitties.
#  This wrapper of ReverseAuction exists only so that users can create
#  auctions with only one transaction.
class KittyAuction(KittyBreeding):

    # @notice The auction contract variables are defined in KittyBase to allow
    #  us to refer to them in KittyOwnership to prevent accidental transfers.
    # `saleAuction` refers to the auction for gen0 and p2p sale of kitties.
    # `siringAuction` refers to the auction for siring rights of kitties.

    # @dev Sets the reference to the sale auction.
    # @param _address - Address of sale contract.
    @onlyCEO
    def setSaleAuctionAddress(_address: address):
        candidateContract = SaleClockAuction(_address)

        # NOTE: verify that a contract is what we expect - https:#github.com/Lunyr/crowdsale-contracts/blob/cfadd15986c30521d8ba7d5b6f57b4fefcc7ac38/contracts/LunyrToken.sol#L117
        require(candidateContract.isSaleClockAuction())

        # Set the new contract address
        self.saleAuction = candidateContract

    # @dev Sets the reference to the siring auction.
    # @param _address - Address of siring contract.
    @onlyCEO
    def setSiringAuctionAddress(_address: address):
        candidateContract = SiringClockAuction(_address)

        # NOTE: verify that a contract is what we expect - https://github.com/Lunyr/crowdsale-contracts/blob/cfadd15986c30521d8ba7d5b6f57b4fefcc7ac38/contracts/LunyrToken.sol#L117
        require(candidateContract.isSiringClockAuction())

        # Set the new contract address
        self.siringAuction = candidateContract;

    # @dev Put a kitty up for auction.
    #  Does some ownership trickery to create auctions in one tx.
    @whenNotPaused
    def createSaleAuction(
        _kittyId: uint256,
        _startingPrice: uint256,
        _endingPrice: uint256,
        _duration: uint256):
        # Auction contract checks input sizes
        # If kitty is already on any auction, this will throw
        # because it will be owned by the auction contract.
        require(_owns(msg.sender, _kittyId));
        # Ensure the kitty is not pregnant to prevent the auction
        # contract accidentally receiving ownership of the child.
        # NOTE: the kitty IS allowed to be in a cooldown.
        require(!isPregnant(_kittyId));
        self._approve(_kittyId, saleAuction);
        # Sale auction throws if inputs are invalid and clears
        # transfer and sire approval after escrowing the kitty.
        saleAuction.createAuction(
            _kittyId,
            _startingPrice,
            _endingPrice,
            _duration,
            msg.sender)

    # @dev Put a kitty up for auction to be sire.
    #  Performs checks to ensure the kitty can be sired, then
    #  delegates to reverse auction.
    @whenNotPaused
    def createSiringAuction(
        _kittyId: uint256,
        _startingPrice: uint256,
        _endingPrice: uint256,
        _duration: uint256):
        # Auction contract checks input sizes
        # If kitty is already on any auction, this will throw
        # because it will be owned by the auction contract.
        require(_owns(msg.sender, _kittyId))
        require(isReadyToBreed(_kittyId))
        self._approve(_kittyId, siringAuction)
        # Siring auction throws if inputs are invalid and clears
        # transfer and sire approval after escrowing the kitty.
        siringAuction.createAuction(
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
    def bidOnSiringAuction(_sireId: uint256, _matronId: uint256):
        # Auction contract checks input sizes
        require(_owns(msg.sender, _matronId));
        require(self.isReadyToBreed(_matronId));
        require(self._canBreedWithViaAuction(_matronId, _sireId));

        # Define the current price of the auction.
        currentPrice = self.siringAuction.getCurrentPrice(_sireId);
        require(msg.value >= currentPrice + self.autoBirthFee);

        # Siring auction will throw if the bid fails.
        self.siringAuction.bid.value(msg.value - autoBirthFee)(_sireId);
        self._breedWith(uint32(_matronId), uint32(_sireId));

    # @dev Transfers the balance of the sale auction contract
    # to the KittyCore contract. We use two-step withdrawal to
    # prevent two transfer calls in the auction bid function.
    @onlyCLevel
    def withdrawAuctionBalances():
        saleAuction.withdrawBalance()
        siringAuction.withdrawBalance()

# @title all functions related to creating kittens
class KittyMinting(KittyAuction):
    def __init__(self):
        super(KittyAuction, self).__init__()
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
        self.GEN0_STARTING_PRICE = 10 #finney;
        self.GEN0_AUCTION_DURATION = 1 #days;

    # Counts the number of cats the contract owner has created.
        self.promoCreatedCount = uint256(0)
        self.en0CreatedCount = uint256(0)

    # @dev we can create promo kittens, up to a limit. Only callable by COO
    # @param _genes the encoded genes of the kitten to be created, any value is accepted
    # @param _owner the future owner of the created kittens. Default to contract COO
    @onlyCOO
    def createPromoKitty(_genes: uint256, _owner: address):
        kittyOwner = _owner;
        if kittyOwner == address(0):
             kittyOwner = cooAddress
        require(self.promoCreatedCount < PROMO_CREATION_LIMIT)
        self.promoCreatedCount++;
        self._createKitty(0, 0, 0, _genes, kittyOwner)

    # @dev Creates a new gen0 kitty with the given genes and
    #  creates an auction for it.
    @onlyCOO
    def createGen0Auction(_genes: uint256):
        require(gen0CreatedCount < GEN0_CREATION_LIMIT)

        kittyId = self._createKitty(0, 0, 0, _genes, address(this))
        self._approve(kittyId, saleAuction)

        saleAuction.createAuction(
            kittyId,
            _computeNextGen0Price(),
            0,
            GEN0_AUCTION_DURATION,
            address(this)
        );
        self.gen0CreatedCount++;

    # @dev Computes the next gen0 auction starting price, given
    #  the average of the past 5 prices + 50%.
    def _computeNextGen0Price() -> uint256:
        avePrice = self.saleAuction.averageGen0SalePrice()

        # Sanity check to ensure we don't overflow arithmetic
        require(avePrice == uint256(uint128(avePrice)))

        nextPrice = avePrice + (avePrice / 2)

        # We never auction for less than starting price
        if nextPrice < GEN0_STARTING_PRICE:
            nextPrice = GEN0_STARTING_PRICE;

        return nextPrice;


# @title CryptoKitties: Collectible, breedable, and oh-so-adorable cats on the Ethereum blockchain.
# @author Axiom Zen (https://www.axiomzen.co)
# @dev The main CryptoKitties contract, keeps track of kittens so they don't wander around and get lost.
class KittyCore(KittyMinting):

    # This is the main CryptoKitties contract. In order to keep our code seperated into logical sections,
    # we've broken it up in two ways. First, we have several seperately-instantiated sibling contracts
    # that handle auctions and our super-top-secret genetic combination algorithm. The auctions are
    # seperate since their logic is somewhat complex and there's always a risk of subtle bugs. By keeping
    # them in their own contracts, we can upgrade them without disrupting the main contract that tracks
    # kitty ownership. The genetic combination algorithm is kept seperate so we can open-source all of
    # the rest of our code without making it _too_ easy for folks to figure out how the genetics work.
    # Don't worry, I'm sure someone will reverse engineer it soon enough!
    #
    # Secondly, we break the core contract into multiple files using inheritence, one for each major
    # facet of functionality of CK. This allows us to keep related code bundled together while still
    # avoiding a single giant file with everything in it. The breakdown is as follows:
    #
    #      - KittyBase: This is where we define the most fundamental code shared throughout the core
    #             functionality. This includes our main data storage, constants and data types, plus
    #             internal functions for managing these items.
    #
    #      - KittyAccessControl: This contract manages the various addresses and constraints for operations
    #             that can be executed only by specific roles. Namely CEO, CFO and COO.
    #
    #      - KittyOwnership: This provides the methods required for basic non-fungible token
    #             transactions, following the draft ERC-721 spec (https:#github.com/ethereum/EIPs/issues/721).
    #
    #      - KittyBreeding: This file contains the methods necessary to breed cats together, including
    #             keeping track of siring offers, and relies on an external genetic combination contract.
    #
    #      - KittyAuctions: Here we have the public methods for auctioning or bidding on cats or siring
    #             services. The actual auction functionality is handled in two sibling contracts (one
    #             for sales and one for siring), while auction creation and bidding is mostly mediated
    #             through this facet of the core contract.
    #
    #      - KittyMinting: This final facet contains the functionality we use for creating new gen0 cats.
    #             We can make up to 5000 "promo" cats that can be given away (especially important when
    #             the community is new), and all others can only be created and then immediately put up
    #             for auction via an algorithmically determined starting price. Regardless of how they
    #             are created, there is a hard limit of 50k gen0 cats. After that, it's all up to the
    #             community to breed, breed, breed!

    # Set in case the core contract is broken and an upgrade is required

    # @notice Creates the main CryptoKitties smart contract instance.
    def __init__():
        super(KittyMinting, self).__init__()
        self.newContractAddress = address(0)
        # Starts paused.
        self.paused = True;

        # the creator of the contract is the initial CEO
        self.ceoAddress = msg.sender;

        # the creator of the contract is also the initial COO
        self.cooAddress = msg.sender;

        # start with the mythical kitten 0 - so we don't have generation-0 parent issues
        self._createKitty(0, 0, 0, uint256(-1), address(0));

    # @dev Used to mark the smart contract as upgraded, in case there is a serious
    #  breaking bug. This method does nothing but keep track of the new contract and
    #  emit a message indicating that the new address is set. It's up to clients of this
    #  contract to update to the new contract address in that case. (This contract will
    #  be paused indefinitely if such an upgrade takes place.)
    # @param _v2Address new address
    @onlyCEO
    @whenPaused
    def setNewAddress(self, _v2Address: address):
        # See README.md for updgrade plan
        newContractAddress = _v2Address
        ContractUpgrade(_v2Address)

    # @notice No tipping!
    # @dev Reject all Ether from being sent here, unless it's from one of the
    #  two auction contracts. (Hopefully, we can prevent user accidents.)
    def __call__(self): # external payable {
        require(msg.sender == address(saleAuction) or msg.sender == address(siringAuction) )

    # @notice Returns all the relevant information about a specific kitty.
    # @param _id The ID of the kitty of interest.
    def getKitty(self, uint256 _id):
        kit = kitties[_id]
        # if this variable is 0 then it's not gestating
        isGestating = (kit.siringWithId != 0)
        isReady = (kit.cooldownEndBlock <= block.number)
        cooldownIndex = uint256(kit.cooldownIndex)
        nextActionAt = uint256(kit.cooldownEndBlock)
        siringWithId = uint256(kit.siringWithId)
        birthTime = uint256(kit.birthTime)
        matronId = uint256(kit.matronId)
        sireId = uint256(kit.sireId)
        generation = uint256(kit.generation)
        genes = kit.genes

        returns (
            isGestating,
            isReady,
            cooldownIndex,
            nextActionAt,
            siringWithId,
            birthTime,
            matronId,
            sireId,
            generation,
            genes)

    # @dev Override unpause so it requires all external contract addresses
    #  to be set before contract can be unpaused. Also, we can't have
    #  newContractAddress set either, because then the contract was upgraded.
    # @notice This is public rather than external so we can call super.unpause
    #  without using an expensive CALL.
    @onlyCEO
    @whenPaused
    def unpause(self):
        require(saleAuction != address(0))
        require(siringAuction != address(0))
        require(geneScience != address(0))
        require(newContractAddress == address(0))
        # Actually unpause the contract.
        super.unpause()

    # @dev Allows the CFO to capture the balance available to the contract.
    @onlyCFO
    def withdrawBalance(self):
        balance = this.balance
        # Subtract all the currently pregnant kittens we have, plus 1 of margin.
        subtractFees = (self.pregnantKitties + 1) * self.autoBirthFee

        if balance > subtractFees:
            cfoAddress.send(balance - subtractFees)
