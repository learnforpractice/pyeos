from backend import *
#/ @title SEKRETOOOO
class GeneScienceInterface:
    #/ @dev simply a boolean to indicate this is the contract we expect to be
    def isGeneScience(self) -> bool:
        pass

    #/ @dev given genes of kitten 1 & 2, return a genetic combination - may have a random factor
    #/ @param genes1 genes of mom
    #/ @param genes2 genes of sire
    #/ @return the genes that are supposed to be passed down the child
    def mixGenes(self, genes1: uint256, genes2: uint256, targetBlock: uint256) ->uint256:
        pass

        