import db
import ustruct
from eoslib import *
'''
msg.data (bytes): complete calldata
msg.gas (uint): remaining gas - deprecated in version 0.4.21 and to be replaced by gasleft()
msg.sender (address): sender of the message (current call)
msg.sig (bytes4): first four bytes of the calldata (i.e. function identifier)
msg.value (uint): number of wei sent with the message
'''

contract_owner = N('kitties')
receiver = current_receiver()
g_table_id = N('tableid')

class baseint(object):
    def __init__(self, v):
        if isinstance(v, int):
            self._v = v
        elif isinstance(v, baseint):
            self._v = v._v
        else:
            raise Exception("unknown type")

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

    def __truediv__(self, other):
        if isinstance(other, int):
            self._v = int(self._v / other)
        else:
            self._v = int(self._v / other._v)
        return self.__class__(self._v)

    def __eq__(self, other):
        if isinstance(other, int):
            return self._v == other
        else:
            return self._v == other._v

    def __gt__(self,other):
        if isinstance(other, int):
            return self._v > other
        else:
            return self._v > other._v

    def __ge__(self,other):
        if isinstance(other, int):
            return self._v >= other
        else:
            return self._v >= other._v

    def __lt__(self,other):
        if isinstance(other, int):
            return self._v < other
        else:
            return self._v < other._v

    def __le__(self,other):
        if isinstance(other, int):
            return self._v <= other
        else:
            return self._v <= other._v

    def __repr__(self):
        return '%s(%s)' % (type(self).__name__, str(self._v))

class uint16(baseint):
    pass

class uint(baseint):
    pass

class uint32(baseint):
    pass

class uint64(baseint):
    pass

class uint128(baseint):
    pass

class int256(baseint):
    pass

class uint256(baseint):
    pass

class address(int):
    def __init__(self, value):
        self.balance = value
    
    def transfer(self, _from, _to):
        pass

    def ownerOf(self,address):
        pass
    
    def send(self):
        pass

def balanceOf(address):
    return 99999

def require(condition, msg = ''):
    print("++++++++++++++require:",condition)
    if not condition:
        raise Exception(msg)
    #eosio_assert(condition, msg)

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
    return 0;
#    return sha256(_str)

class bytes4(object):
    def __init__(self, value):
        return
        if isinstance(value, str):
            self._v = bytes(value, 'utf8')
        elif isinstance(value, bytes):
            self._v = value
        else:
            raise Exception("unsupported type " + str(value))

class bytes32(bytes):
    pass

class List(object):

    def __init__(self, size = 0, value = [], value_type = object):
        if value:
            eosio_assert(isinstance(value, list), "bad type")
            self._list = value
        else:
            self._list = [None for i in range(size)]

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

def get_hash(v):
    if type(v) is int:
        return v
    elif type(v) in (str, bytes):
        return hash64(v)

def to_raw_value(value):
    if isinstance(value, int):
        _value = b'\x00\x08\x00'
        _value += int.to_bytes(value, 8, 'little')
        return _value
    elif isinstance(value, str):
        _value = b'\x01'
        _value += int.to_bytes(len(value), 2, 'little')
        _value += value.encode('utf8')
        return _value
    else:
        require(False, 'unkonwn type')

def from_raw_value(value):
    require(isinstance(value, str) or isinstance(value, bytes))
    type_id = int.from_bytes(value[0], 'little')
    if isinstance(value, int):
        return ustruct.pack('BQ', 8, value)
    elif isinstance(value, str):
        _value = ustruct.pack('B', len(value))
        _value += value.encode('utf8')
        return _value
    else:
        require(False, 'unkonwn type')

def store(key, value):
    id = get_hash(key)
    value = to_raw_value(value)
    code = contract_owner
    itr = db.find_i64(receiver, receiver, g_table_id, id)
    if itr >= 0:
        value = db.update_i64(itr, receiver, value)
    else:
        db.store_i64(receiver, g_table_id, receiver, id, value)

def load(key):
    id = get_hash(key)
    itr = db.find_i64(receiver, receiver, g_table_id, id)
    value = None
    if itr >= 0:
        value = db.get_i64(itr)
        return from_raw_value(value)
    return None

this = address(991)

msg = message('',0,'110',0,0)

