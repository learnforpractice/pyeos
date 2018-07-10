from backend import *
# @title The external contract that is responsible for generating metadata for the kitties,
#  it has one function that will return the data as bytes.
class ERC721Metadata:
    # @dev Given a token Id, returns a byte array that is supposed to be converted into string.
    def getMetadata(self, _tokenId: uint256) -> (List, uint256):
        buffer = List(size = 4, value_type=str)
        count = 0
        if _tokenId == 1:
            buffer[0] = "Hello World! :D";
            count = 15;
        elif _tokenId == 2:
            buffer[0] = "I would definitely choose a medi";
            buffer[1] = "um length string.";
            count = 49;
        elif _tokenId == 3:
            buffer[0] = "Lorem ipsum dolor sit amet, mi e";
            buffer[1] = "st accumsan dapibus augue lorem,";
            buffer[2] = " tristique vestibulum id, libero";
            buffer[3] = " suscipit varius sapien aliquam.";
            count = 128;
        return (buffer, uint256(count))
    