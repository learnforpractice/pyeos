#migrate from http:#solidity.readthedocs.io/en/latest/solidity-by-example.html
# @title Voting with delegation.
import ustruct as struct
from eoslib import *
#from storage import *
import storage
print(storage.__path__)
print(dir(storage))

code = N('vote')
scope = code
payer = code

def require(condition, msg = ''):
    print("++++++++++++++require:",condition)
    eosio_assert(condition, msg)

class Voter(object):
    def __init__(self):
        self.weight = 0; # weight is accumulated by delegation
        self.voted = 0;  # if true, that person already voted
        self.delegate = 0; # person delegated to
        self.vote = 0;   # index of the voted proposal

    def pack(self):
        return struct.pack('QBQB', self.weight, self.voted, self.delegate, self.vote)

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

    def pack(self):
        data = int.to_bytes(voteCount, 4, 'little')
        return data + self.name

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
    def __init__(self):
        self.sender = current_sender()
        
        self.chairperson = 0;
        self.voters = SDict(table_id=1, value_type=Voter)
        self.proposals = SList(table_id=2, value_type=Proposal)

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
        require_auth(N('vote'))
        require(not voters[voter].voted,"The voter already voted.")
        require(voters[voter].weight == 0)
        voters[voter].weight = 1;

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
        require(to != sender, "Self-delegation is disallowed.");
        # assigns reference
        voter = self.voters[sender]
        require(not voter.voted, "You already voted.");

        # Forward the delegation as long as
        # `to` also delegated.
        # In general, such loops are very dangerous,
        # because if they run too long, they might
        # need more gas than is available in a block.
        # In this case, the delegation will not be executed,
        # but in other situations, such loops might
        # cause a contract to get "stuck" completely.
        while self.voters[to].delegate != 0:
            to = self.voters[to].delegate
            # We found a loop in the delegation, not allowed.
            require(to != sender, "Found loop in delegation.")

        # Since `sender` is a reference, this
        # modifies `voters[msg.sender].voted`
        voter.voted = True
        voter.delegate = to

        #-- save changed voter
        self.voters[sender] = voter
        
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
    def vote(proposal):
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

def apply(name, action):
    if action == N('deploy'):
        require_auth(code)
        msg = read_action()
        length = int.from_bytes(msg[:1], 'little')
        mod_name = msg[1:1+length]
        src_code = msg[1+length:]
        print('+++++++++++++++++src_code type:', src_code[0])
        deploy(mod_name, src_code)
    elif action == N('addproposal'):
        b = Ballot()
        account = read_action()
        b.addProposal(account)
    elif action == N('giveright'):
        b = Ballot()
        account = read_action()
        account = N(account)
        b.giveRightToVote(account)
    elif action == N('delegate'):
        b = Ballot()
        account = read_action()
        account = N(account)
        b.delegate(account)
    elif action == N('vote'):
        b = Ballot()
        account = read_action()
        account = N(account)
        b.vote(account)

