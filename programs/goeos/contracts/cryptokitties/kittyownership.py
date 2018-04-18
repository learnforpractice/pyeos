from backend import *
from basement import *
from pausable import *
from kittyaccesscontrol import *

from kittybase import KittyBase
from erc721 import ERC721
from erc721metadata import ERC721Metadata

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

        '''FIXME
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

    def supportsInterface(self, _interfaceID: bytes) -> bool:
        # DEBUG ONLY
        #require((InterfaceSignature_ERC165 == 0x01ffc9a7) && (InterfaceSignature_ERC721 == 0x9a20483d));
        return (_interfaceID == self.InterfaceSignature_ERC165) or (_interfaceID == self.InterfaceSignature_ERC721)


    # @dev Set the address of the sibling contract that tracks metadata.
    #  CEO only.
    @onlyCEO
    def setMetadataAddress(self, _contractAddress: address):
        self.erc721Metadata = ERC721Metadata(_contractAddress)

    # Internal utility functions: These functions all assume that their input arguments
    # are valid. We leave it to public methods to sanitize their inputs and follow
    # the required logic.

    # @dev Checks if a given address is the current owner of a particular Kitty.
    # @param _claimant the address we are validating against.
    # @param _tokenId kitten id, only valid when > 0
    def _owns(self, _claimant: address, _tokenId: uint256) -> bool:
        return self.kittyIndexToOwner[_tokenId] == _claimant

    # @dev Checks if a given address currently has transferApproval for a particular Kitty.
    # @param _claimant the address we are confirming kitten is approved for.
    # @param _tokenId kitten id, only valid when > 0
    def _approvedFor(self, _claimant: address, _tokenId: uint256) -> bool:
        return self.kittyIndexToApproved[_tokenId] == _claimant

    # @dev Marks an address as being approved for transferFrom(), overwriting any previous
    #  approval. Setting _approved to address(0) clears all transfer approval.
    #  NOTE: _approve() does NOT send the Approval event. This is intentional because
    #  _approve() and transferFrom() are used together for putting Kitties on auction, and
    #  there is no value in spamming the log with Approval events in that case.
    def _approve(self, _tokenId: uint256, _approved: address):
        self.kittyIndexToApproved[_tokenId] = _approved

    # @notice Returns the number of Kitties owned by a specific address.
    # @param _owner The owner address to check.
    # @dev Required for ERC-721 compliance
    def balanceOf(self, _owner: address)  -> uint256:
        return self.ownershipTokenCount[_owner]
    

    # @notice Transfers a Kitty to another address. If transferring to a smart
    #  contract be VERY CAREFUL to ensure that it is aware of ERC-721 (or
    #  CryptoKitties specifically) or your Kitty may be lost forever. Seriously.
    # @param _to The address of the recipient, can be a user or contract.
    # @param _tokenId The ID of the Kitty to transfer.
    # @dev Required for ERC-721 compliance.
    def transfer(self, _to: address, _tokenId: uint256):
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
        require(_to != address(self.saleAuction))
        require(_to != address(self.siringAuction))
        # You can only send your own cat.
        require(self._owns(msg.sender, _tokenId))
        # Reassign ownership, clear pending approvals, emit Transfer event.
        self._transfer(msg.sender, _to, _tokenId)

    # @notice Grant another address the right to transfer a specific Kitty via
    #  transferFrom(). This is the preferred flow for transfering NFTs to contracts.
    # @param _to The address to be granted transfer approval. Pass address(0) to
    #  clear all approvals.
    # @param _tokenId The ID of the Kitty that can be transferred if this call succeeds.
    # @dev Required for ERC-721 compliance.
    @whenNotPaused
    def approve(self, _to: address, _tokenId: uint256):
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
    @whenNotPaused
    def transferFrom(self, _from: address, _to: address, _tokenId: uint256):
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
    def totalSupply(self) -> uint:
        return self.kitties.length - 1

    # @notice Returns the address currently assigned ownership of a given Kitty.
    # @dev Required for ERC-721 compliance.
    def ownerOf(self, _tokenId: uint256) -> address:
        owner = self.kittyIndexToOwner[_tokenId]
        require(owner != address(0))
        return owner

    # @notice Returns a list of all Kitty IDs assigned to an address.
    # @param _owner The owner whose Kitties we are interested in.
    # @dev This method MUST NEVER be called by smart contract code. First, it's fairly
    #  expensive (it walks the entire Kitty array looking for cats belonging to owner),
    #  but it also returns a dynamic array, which is only supported for web3 calls, and
    #  not contract-to-contract calls.
    def tokensOfOwner(self, _owner: address) -> List:
        tokenCount = self.balanceOf(_owner)

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
            totalCats = self.totalSupply()
            resultIndex = 0

            # We count on the fact that all cats have IDs starting at 1 and increasing
            # sequentially up to the totalCat count.
#            uint256 catId;

            for catId in range(1, totalCats+1):
                if self.kittyIndexToOwner[catId] == _owner:
                    result[resultIndex] = catId
                    resultIndex+=1

            return result

    # @dev Adapted from memcpy() by @arachnid (Nick Johnson <arachnid@notdot.net>)
    #  This method is licenced under the Apache License.
    #  Ref: https://github.com/Arachnid/solidity-stringutils/blob/2f6ca9accb48ae14c66f1437ec50ed19a0616f78/strings.sol

    def _memcpy(self, _dest: uint, _src: uint, _len: uint):
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
    def _toString(self, _rawBytes, _stringLength) -> str:
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
    def tokenMetadata(self, _tokenId: uint256, _preferredTransport: str) -> str:
        require(self.erc721Metadata != address(0))
#        bytes32[4] memory buffer;
#        uint256 count;
        buffer, count = self.erc721Metadata.getMetadata(_tokenId, _preferredTransport)
        return self._toString(buffer, count)
