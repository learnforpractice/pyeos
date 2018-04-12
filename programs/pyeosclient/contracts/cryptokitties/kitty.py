from backend import *
import ustruct
#/*** DATA TYPES ***/

# @dev The main Kitty struct. Every cat in CryptoKitties is represented by a copy
#  of this structure, so great care was taken to ensure that it fits neatly into
#  exactly two 256-bit words. Note that the order of the members in this structure
#  is important because of the byte-packing rules used by Ethereum.
#  Ref: http://solidity.readthedocs.io/en/develop/miscellaneous.html

class Kitty:
    def __init__(self, _genes=uint256(0), _birthTime=uint64(0), _cooldownEndBlock = uint64(0),
                 _matronId=uint32(0), _sireId=uint32(0), _siringWithId=uint32(0), 
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

    def pack(self):
        genes = ustruct.pack('QQQQ', self.genes)
        return genes + ustruct.pack('QQQQQQQ', self.birthTime, self.cooldownEndBlock,
                            self.matronId, self.sireId, self.siringWithId, self.cooldownIndex, self.generation)

    def unpack(self, value):
        a  = ustruct.unpack('QQQQ', value[:32])
        b, c, d, e, f, g, h = ustruct.unpack('QQQQQQQ', value[32:])
        return Kitty(a, b, c, d, e, f, g, h)

