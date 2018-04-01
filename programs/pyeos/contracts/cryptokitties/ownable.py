from backend import *
from basement import *
class Ownable:
    def __init__(self):
        self.owner = None
        itr = db_find_i64(contract_owner, contract_owner, contract_owner, N('owner'))
        if itr >= 0:
            value = db_get_i64(itr)
            self.owner = address(value)
    @onlyOwner
    def transferOwnership(newOwner: address):
        if newOwner != address(0):
            self.owner = newOwner;
            