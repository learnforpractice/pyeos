#migrate from http:#solidity.readthedocs.io/en/latest/solidity-by-example.html
# @title Voting with delegation.
import ustruct as struct
from eoslib import *
#from storage import *
from backyard.storage import SDict, SList, register_pack_func, register_unpack_func

code = N('vote')
scope = code
payer = code

def require(cond, msg = ''):
    if cond:
        cond = 1
    else:
        cond = 0
    eosio_assert(cond, msg)

class Voter(object):
    def __init__(self):
        self.weight = 0; # weight is accumulated by delegation
        self.voted = 0;  # if true, that person already voted
        self.delegate = 0; # person delegated to
        self.vote = 0;   # index of the voted proposal

    def pack(v):
        return struct.pack('QBQB', v.weight, v.voted, v.delegate, v.vote)

    def unpack(data):
        v = Voter()
        v.weight, v.voted, v.delegate, v.vote = struct.unpack('QBQB', data)
        return v

    def __eq__(self, other):
        if not isinstance(other, Voter):
            return False
        return self.weight == other.weight and \
                self.voted == other.voted and \
                self.delegate == other.delegate and \
                self.vote == other.vote

# This is a type for a single proposal.
class Proposal(object):
    def __init__(self, _name=0, _voteCount=0):
        self.name = _name;   # short name (up to 32 bytes)
        self.voteCount = _voteCount; # number of accumulated votes

    def pack(v):
        data = int.to_bytes(v.voteCount, 4, 'little')
        return data + v.name

    def unpack(data):
        v = Proposal()
        v.voteCount = int.from_bytes(data[:4], 'little')
        v.name = data[4:]
        return v

    def __eq__(self, other):
        if not isinstance(other, Proposal):
            return False
        return self.name == other.name and \
                self.voteCount == other.voteCount

class Ballot(object):
    def __init__(self, sender):
        self.sender = sender
        
        self.chairperson = 0;
        self.voters = SDict(code, table_id=1, value_type=Voter)
        self.proposals = SList(code, table_id=2, value_type=Proposal)

        #FIXME only need to set once
        chairperson = N('vote');
        v = Voter()
        v.weight = 1;
        self.voters[chairperson] = v

    def addProposal(self, name):
        p = Proposal(name, 0)
        self.proposals.append(p)

    # Give `voter` the right to vote on this ballot.
    # May only be called by `chairperson`.
    def giveRightToVote(self, voter):
        require_auth(self.sender)
        if self.voters.find(voter):
            return

        v = Voter()
        v.weight = 1
        # save it by assign a new value
        self.voters[voter] = v

        '''
        require(
            msg.sender == chairperson,
            "Only chairperson can give right to vote."
        );
        require(
            !voters[voter].voted,
            "The voter already voted."
        );
        require(voters[voter].weight == 0);
        voters[voter].weight = 1;
        '''

    # Delegate your vote to the voter `to`.
    def delegate(self, to):
        #not a voter
        if not self.voters.find(to):
            return
#        require(self.voters.find(to), n2s(to) + ' is not a voter')
        require(to != self.sender, "Self-delegation is disallowed.")
        # assigns reference
        voter = self.voters[self.sender]
        require(not voter.voted, "You already voted.")

        # Forward the delegation as long as
        # `to` also delegated.
        # In general, such loops are very dangerous,
        # because if they run too long, they might
        # need more gas than is available in a block.
        # In this case, the delegation will not be executed,
        # but in other situations, such loops might
        # cause a contract to get "stuck" completely.
        while True:
            v = self.voters.find(to)
            if not v:
                break
            if v.delegate == 0:
                break
            to = v.delegate
            # We found a loop in the delegation, not allowed.
            require(to != self.sender, "Found loop in delegation.")

        # Since `sender` is a reference, this
        # modifies `voters[msg.sender].voted`
        voter.voted = True
        voter.delegate = to

        #-- save changed voter
        self.voters[self.sender] = voter
        
        delegate_ = self.voters[to]
        if delegate_.voted:
            # If the delegate already voted,
            # directly add to the number of votes
            self.proposals[delegate_.vote].voteCount += voter.weight;
        else:
            # If the delegate did not vote yet,
            # add to her weight.
            delegate_.weight += voter.weight;

    #/ Give your vote (including votes delegated to you)
    #/ to proposal `proposals[proposal].name`.
    def vote(self, proposal):
        voter = self.voters[self.sender]
        require(not voter.voted, "Already voted.");
        voter.voted = True;
        voter.vote = proposal;

        #-- save voter
        self.voters[self.sender] = voter

        # If `proposal` is out of the range of the array,
        # this will throw automatically and revert all
        # changes.
        self.proposals[proposal].voteCount += voter.weight;

    #/ @dev Computes the winning proposal taking all
    #/ previous votes into account.
    def winningProposal(self):
        winningVoteCount = 0;
        winningProposal_ = 0
        for p in range(len(self.proposals)):
            if self.proposals[p].voteCount > winningVoteCount:
                winningVoteCount = self.proposals[p].voteCount
                winningProposal_ = p
        return winningProposal_

    # Calls winningProposal() function to get the index
    # of the winner contained in the proposals array and then
    # returns the name of the winner
    def winnerName(self):
        index = self.winningProposal()
        return self.proposals[index].name;

def deploy(mod_name, src_code):
    print('++++++++++++deploy:mod_name', mod_name)
    id = hash64(mod_name)
    itr = db_find_i64(code, code, code, id)
    if itr < 0:
        db_store_i64(code, code, code, id, src_code)
    else:
        db_update_i64(itr, code, src_code)

    if mod_name.endswith('.mpy'):
        #try to remove py source to prevent micropython loading the wrong source
        mod_name = mod_name[:-3] + 'py'
        id = hash64(mod_name)
        itr = db_find_i64(code, code, code, id)
        if itr >= 0:
            db_remove_i64(itr)

def delegate(name=None):
        msg = struct.pack('QQ', eosapi.N('vote'), eosapi.N(name))
def vote(voter, proposal_index):
        msg = struct.pack('QQ', eosapi.N(voter), proposal_index)

def apply(receiver, code, action):
    if action == N('deploy'):
        require_auth(code)
        msg = read_action()
        length = int.from_bytes(msg[:1], 'little')
        mod_name = msg[1:1+length]
        src_code = msg[1+length:]
        print('++++++++++++++++ +src_code type:', src_code[0])
        deploy(mod_name, src_code)
    elif action == N('addproposal'):
        msg = read_action()
        assert len(msg) > 8
        sender = int.from_bytes(msg[:8], 'little')
        proposal = msg[8:].decode('utf8')
        b = Ballot(sender)
        b.addProposal(proposal)
    elif action == N('giveright'):
        msg = read_action()
        sender = int.from_bytes(msg[:8], 'little')
        who = int.from_bytes(msg[8:], 'little')
        b = Ballot(sender)
        b.giveRightToVote(who)
    elif action == N('delegate'):
        msg = read_action()
        sender = int.from_bytes(msg[:8], 'little')
        who = int.from_bytes(msg[8:], 'little')
        b = Ballot(sender)
        b.delegate(who)
    elif action == N('vote'):
        msg = read_action()
        sender = int.from_bytes(msg[:8], 'little')
        proposal_index = int.from_bytes(msg[8:], 'little')
        b = Ballot(sender)
        b.vote(proposal_index)

