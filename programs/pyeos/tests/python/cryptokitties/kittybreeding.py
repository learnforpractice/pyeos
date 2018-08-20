from backend import *
from basement import *
from kittyaccesscontrol import *

from kitty import Kitty
from kittyownership import KittyOwnership
from genescienceinterface import GeneScienceInterface

# @title A facet of KittyCore that manages Kitty siring, gestation, and birth.
# @author Axiom Zen (https://www.axiomzen.co)
# @dev See the KittyCore contract documentation to understand how the various contract facets are arranged.
class KittyBreeding(KittyOwnership):
    def __init__(self):
        super(KittyBreeding, self).__init__()
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
    @event
    def Pregnant(self, owner: address, matronId: uint256, sireId: uint256, cooldownEndBlock: uint256):
        raise Exception('Fix event')

    # @dev Update the address of the genetic contract, can only be called by the CEO.
    # @param _address An address of a GeneScience contract instance to be used from this point forward.
    @onlyCEO
    def setGeneScienceAddress(self, _address: address):
        candidateContract = GeneScienceInterface(_address)

        # NOTE: verify that a contract is what we expect - https://github.com/Lunyr/crowdsale-contracts/blob/cfadd15986c30521d8ba7d5b6f57b4fefcc7ac38/contracts/LunyrToken.sol#L117
        require(candidateContract.isGeneScience());

        # Set the new contract address
        geneScience = candidateContract;

    # @dev Checks that a given kitten is able to breed. Requires that the
    #  current cooldown is finished (for sires) and also checks that there is
    #  no pending pregnancy.
    def _isReadyToBreed(self, _kit: Kitty) -> bool:
        # In addition to checking the cooldownEndBlock, we also need to check to see if
        # the cat has a pending birth; there can be some period of time between the end
        # of the pregnacy timer and the birth event.
        return (_kit.siringWithId == 0) and (_kit.cooldownEndBlock <= uint64(block.number));


    # @dev Check if a sire has authorized breeding with this matron. True if both sire
    #  and matron have the same owner, or if the sire has given siring permission to
    #  the matron's owner (via approveSiring()).
    def _isSiringPermitted(self, _sireId: uint256, _matronId: uint256) -> bool:
        matronOwner = self.kittyIndexToOwner[_matronId]
        sireOwner = self.kittyIndexToOwner[_sireId]

        # Siring is okay if they have same owner, or if the matron's owner was given
        # permission to breed with this sire.
        return matronOwner == sireOwner or self.sireAllowedToAddress[_sireId] == matronOwner

    # @dev Set the cooldownEndTime for the given Kitty, based on its current cooldownIndex.
    #  Also increments the cooldownIndex (unless it has hit the cap).
    # @param _kitten A reference to the Kitty in storage which needs its timer started.
    #def _triggerCooldown(Kitty storage _kitten) internal {

    def _triggerCooldown(self, _kitten: Kitty):
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
    @whenNotPaused
    def approveSiring(self, _addr: address, _sireId: uint256):
        require(self._owns(msg.sender, _sireId))
        self.sireAllowedToAddress[_sireId] = _addr

    # @dev Updates the minimum payment required for calling giveBirthAuto(). Can only
    #  be called by the COO address. (This fee is used to offset the gas cost incurred
    #  by the autobirth daemon).
    @onlyCOO
    def setAutoBirthFee(self, val: uint256):
        self.autoBirthFee = val;

    # @dev Checks to see if a given Kitty is pregnant and (if so) if the gestation
    #  period has passed.
    def _isReadyToGiveBirth(self, _matron: Kitty) -> bool:
        return (_matron.siringWithId != 0) and (_matron.cooldownEndBlock <= uint64(block.number))

    # @notice Checks that a given kitten is able to breed (i.e. it is not pregnant or
    #  in the middle of a siring cooldown).
    # @param _kittyId reference the id of the kitten, any user can inquire about it
    def isReadyToBreed(self, _kittyId: uint256) -> bool:
        require(_kittyId > 0)
#        Kitty storage kit = kitties[_kittyId];
        kit = self.kitties[_kittyId]
        return self._isReadyToBreed(kit)

    # @dev Checks whether a kitty is currently pregnant.
    # @param _kittyId reference the id of the kitten, any user can inquire about it
    def isPregnant(self, _kittyId: uint256) -> bool:
        require(_kittyId > 0)
        # A kitty is pregnant if and only if this field is set
        return self.kitties[_kittyId].siringWithId != 0;

    # @dev Internal check to see if a given sire and matron are a valid mating pair. DOES NOT
    #  check ownership permissions (that is up to the caller).
    # @param _matron A reference to the Kitty struct of the potential matron.
    # @param _matronId The matron's ID.
    # @param _sire A reference to the Kitty struct of the potential sire.
    # @param _sireId The sire's ID
    def _isValidMatingPair(self, _matron: Kitty, _matronId: uint256, _sire: Kitty, _sireId: uint256) -> bool:
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
    def _canBreedWithViaAuction(self, _matronId: uint256, _sireId: uint256) -> bool:
        matron = self.kitties[_matronId]
        sire = self.kitties[_sireId]
        return self._isValidMatingPair(matron, _matronId, sire, _sireId)

    # @notice Checks to see if two cats can breed together, including checks for
    #  ownership and siring approvals. Does NOT check that both cats are ready for
    #  breeding (i.e. breedWith could still fail until the cooldowns are finished).
    #  TODO: Shouldn't this check pregnancy and cooldowns?!?
    # @param _matronId The ID of the proposed matron.
    # @param _sireId The ID of the proposed sire.
    def canBreedWith(self, _matronId: uint256, _sireId: uint256) -> bool :
        require(_matronId > 0)
        require(_sireId > 0)
        matron = self.kitties[_matronId]
        sire = self.kitties[_sireId]
        return self._isValidMatingPair(matron, _matronId, sire, _sireId) and self._isSiringPermitted(_sireId, _matronId)

    # @dev Internal utility function to initiate breeding, assumes that all breeding
    #  requirements have been checked.
    def _breedWith(self, _matronId: uint256, _sireId: uint256):
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
        del self.sireAllowedToAddress[_matronId]
        del self.sireAllowedToAddress[_sireId]

        # Every time a kitty gets pregnant, counter is incremented.
        self.pregnantKitties += 1

        # Emit the pregnancy event.
        self.Pregnant(self.kittyIndexToOwner[_matronId], _matronId, _sireId, matron.cooldownEndBlock)

    # @notice Breed a Kitty you own (as matron) with a sire that you own, or for which you
    #  have previously been given Siring approval. Will either make your cat pregnant, or will
    #  fail entirely. Requires a pre-payment of the fee given out to the first caller of giveBirth()
    # @param _matronId The ID of the Kitty acting as matron (will end up pregnant if successful)
    # @param _sireId The ID of the Kitty acting as sire (will begin its siring cooldown if successful)
    @whenNotPaused
    def breedWithAuto(self, _matronId: uint256, _sireId: uint256):
        # Checks for payment.
        require(msg.value >= self.autoBirthFee);

        # Caller must own the matron.
        require(self._owns(msg.sender, _matronId));

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
        require(self._isSiringPermitted(_sireId, _matronId))

        # Grab a reference to the potential matron
        matron = self.kitties[_matronId]

        # Make sure matron isn't pregnant, or in the middle of a siring cooldown
        require(self._isReadyToBreed(matron))

        # Grab a reference to the potential sire
        sire = self.kitties[_sireId]

        # Make sure sire isn't pregnant, or in the middle of a siring cooldown
        require(self._isReadyToBreed(sire))

        # Test that these cats are a valid mating pair.
        require(self._isValidMatingPair(matron, _matronId, sire,_sireId))

        # All checks passed, kitty gets pregnant!
        self._breedWith(_matronId, _sireId)

    # @notice Have a pregnant Kitty give birth!
    # @param _matronId A Kitty ready to give birth.
    # @return The Kitty ID of the new kitten.
    # @dev Looks at a given Kitty and, if pregnant and if the gestation period has passed,
    #  combines the genes of the two parents to create a new kitten. The new Kitty is assigned
    #  to the current owner of the matron. Upon successful completion, both the matron and the
    #  new kitten will be ready to breed again. Note that anyone can call this function (if they
    #  are willing to pay the gas!), but the new kitten always goes to the mother's owner.
    @whenNotPaused
    def giveBirth(self, _matronId: uint256) -> uint256:
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
        owner = self.kittyIndexToOwner[_matronId]
        kittenId = self._createKitty(_matronId, matron.siringWithId, parentGen + 1, childGenes, owner)

        # Clear the reference to sire from the matron (REQUIRED! Having siringWithId
        # set is what marks a matron as being pregnant.)
        del matron.siringWithId

        # Every time a kitty gives birth counter is decremented.
        self.pregnantKitties-=1

        # Send the balance fee to the person who made birth happen.
        msg.sender.send(self.autoBirthFee);

        # return the new kitten's ID
        return kittenId;
    