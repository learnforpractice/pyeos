from backend import *
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
    