from eoslib import *
'''
msg.data (bytes): complete calldata
msg.gas (uint): remaining gas - deprecated in version 0.4.21 and to be replaced by gasleft()
msg.sender (address): sender of the message (current call)
msg.sig (bytes4): first four bytes of the calldata (i.e. function identifier)
msg.value (uint): number of wei sent with the message
'''

contract_owner = N('cryptokittie') #sorry kitty, max 12 charactor support

def event(func):
   def func_wrapper(self, *args):
       print('TODO: event')
       return func(self, *args)
   return func_wrapper

def balanceOf(address):
    return 99999

def require(condition, msg = ''):
    eosio_assert(condition)

class message:
    def __init__(self, data, gas, sender, sig, value):
        self.data = data
        self.gas = gas
        self.sender = sender
        self.sig = sig
        self.value = value
#msg.sender.send

'''
block.blockhash(uint blockNumber) returns (bytes32): hash of the given block - only works for 256 most recent blocks excluding current
block.coinbase (address): current block miner’s address
block.difficulty (uint): current block difficulty
block.gaslimit (uint): current block gaslimit
block.number (uint): current block number
block.timestamp (uint): current block timestamp as seconds since unix epoch
'''

class block:
    blockhash = None
    coinbase = None
    difficulty = None
    gaslimit = None
    number = None
    timestamp = None

def keccak256(_str):
    return sha256(_str)

class bytes4(object):
    def __init__(self, value):
        if isinstance(value, str):
            self._v = bytes(value, 'utf8')
        elif isinstance(value, bytes):
            self._v = value
        else:
            raise Exception("unsupported type")

class bytes32(bytes):
    pass

class List(object):
    def __init__(self, size = 0, value = [], value_type = object):
        if value:
            self._list = value
        else:
            self._list = [None for i in range(size)]
        '''
        eosio_assert(isinstance(value, list), "bad type")
        self._list = value
        self.value_type = value_type
        '''
        
    def __getitem__(self, index):
        return self._list[index]

    def __setitem__(self, index, val):
        eosio_assert(isinstance(val, self.value_type), 'bad value type')
        self._list[index] = val

    def push(self, value):
        self._list.append(value)
        return len(self._list)

    @property
    def length(self):
        return len(self._list)

class Dict(object):
    def __init__(self, value = {}, key_type = object, value_type = object):
        eosio_assert(isinstance(value, list), "bad type")
        self._dict = value
        self.key_type = key_type
        self.value_type = value_type

    def __getitem__(self, key):
        eosio_assert(isinstance(key, self.key_type), 'bad key type')
        return self._dict[key]

    def __setitem__(self, key, val):
        eosio_assert(isinstance(key, self.key_type), 'bad key type')
        eosio_assert(isinstance(val, self.value_type), 'bad value type')

        self._list[key] = val

class uint16(int):
    pass

class uint(int):
    pass

class uint32(int):
    pass

class uint64(int):
    pass

class uint128(int):
    pass


class uint256(object):
    def __init__(self, v):
        assert isinstance(v, int)
        self._v = v

    def __add__(self, other):
        if isinstance(other, int):
            self._v += other
        else:
            self._v += other._v
        return self.__class__(self._v)

    def __sub__(self, other):
        if isinstance(other, int):
            self._v -= other
        else:
            self._v -= other._v
        return self.__class__(self._v)

    def __mul__(self, other):
        if isinstance(other, int):
            self._v *= other
        else:
            self._v *= other._v
        return self.__class__(self._v)

    def __div__(self, other):
        if isinstance(other, int):
            self._v /= other
        else:
            self._v /= other._v
        return self.__class__(self._v)

    def __truediv__(self,other):
        if isinstance(other, int):
            self._v = int(self._v / other)
        else:
            self._v = int(self._v / other._v)
        return self.__class__(self._v)

    def __repr__(self):
        return '%s(%s)' % (type(self).__name__, str(self._v))

class address(int):
    def __init__(self):
        self.balance = 0
    
    def transfer(self, _from, _to):
        pass
    def ownerOf(self,address):
        pass

this = address(991)

msg = message('',0,'110',0,0)

