from backend import *
from basement import *
class ERC721:
    def __init__(self):
        pass
#     Required methods
    def totalSupply(self) -> uint256:
        pass
    def balanceOf(self, _owner: address) -> uint256:
        pass
    def ownerOf(self, _tokenId: uint256) -> address:
        pass
    def approve(self, _to: address, _tokenId: uint256):
        pass
    def transfer(self, _to: address, _tokenId: uint256):
        pass
    def transferFrom(self, _from: address, _to: address, _tokenId: uint256):
        pass

    @event
    def Transfer(self, _from: address, to: address, tokenId: uint256):
        pass

    @event
    def Approval(self, owner: address, approved: address, tokenId: uint256):
        pass

    # Optional
    # function name() public view returns (string name);
    # function symbol() public view returns (string symbol);
    # function tokensOfOwner(address _owner) external view returns (uint256[] tokenIds);
    # function tokenMetadata(uint256 _tokenId, string _preferredTransport) public view returns (string infoUrl);

    # ERC-165 Compatibility (https://github.com/ethereum/EIPs/issues/165)
#    function supportsInterface(bytes4 _interfaceID) external view returns (bool);
    def supportsInterface(self, _interfaceID) -> bool:
        pass
    