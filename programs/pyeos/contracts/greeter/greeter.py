'''
contract Greeter         // The contract definition. A constructor of the same name will be automatically called on contract creation. 
{
    address creator;     // At first, an empty "address"-type variable of the name "creator". Will be set in the constructor.
    string greeting;     // At first, an empty "string"-type variable of the name "greeting". Will be set in constructor and can be changed.

    function Greeter(string _greeting) public   // The constructor. It accepts a string input and saves it to the contract's "greeting" variable.
    {
        creator = msg.sender;
        greeting = _greeting;
    }

    function greet() constant returns (string)          
    {
        return greeting;
    }
    
    function getBlockNumber() constant returns (uint) // this doesn't have anything to do with the act of greeting
    {                                                    // just demonstrating return of some global variable
        return block.number;
    }
    
    function setGreeting(string _newgreeting) 
    {
        greeting = _newgreeting;
    }
    
     /**********
     Standard kill() function to recover funds 
     **********/
    
    function kill()
    { 
        if (msg.sender == creator)  // only allow this action if the account sending the signal is the creator
            suicide(creator);       // kills this contract and sends remaining funds back to creator
    }
}
'''
from eoslib import read_action, N, hash64, db_store_i64, db_update_i64, db_find_i64, db_get_i64
import ustruct as struct

code = N('greeter')

type_map = {int:0, str: 1, bytes: 2}

def pack_int(value):
    return b'\x00' + struct.pack('Q', 0, value)

def unpack_int(value):
    return struct.unpack('Q', value[1:])[0]

def pack_str(value):
    return b'\x01' + value.encode('utf8')

def unpack_str(value):
    return value[1:].decode('utf8')

def pack_bytes(value):
    return b'\x02' + value

def unpack_bytes(value):
    return value[1:]

pack_funcs = {0:pack_int, 1:pack_str, 2: pack_bytes}
unpack_funcs = {0:unpack_int, 1:unpack_str, 2:unpack_bytes}

def get_type_id(value):
    return type_map[type(value)]

def pack(value):
    type_id = get_type_id(value)
    return pack_funcs[type_id](value)

def unpack(value):
    type_id = value[0]
    return unpack_funcs[type_id](value)

def storage_get(key):
    id = hash64(key)
    itr = db_find_i64(code, code, code, id)
    if itr >= 0:
        value = db_get_i64(itr)
        return unpack(value)
    return None

def storage_set(key, value):
    id = hash64(key)
    value = pack(value)
    itr = db_find_i64(code, code, code, id)
    if itr >= 0:
        db_update_i64(itr, code, value)
    else:
        db_store_i64(code, code, code, id, value)

def storage(func):
    def func_wrapper(self, *args):
        return func(self, *args)
    return func_wrapper

class Greeter:
    def __init__(self, _greeting = None):
#        self.creator = msg.sender
        self._greeting = storage_get('greeting')

    @property
    def greeting(self):
        return self._greeting

    @greeting.setter
    def greeting(self, value):
        self._greeting = value
        storage_set('greeting', value)

    def kill(self):
        pass
#FIXME
#        if msg.sender == self.creator:
#            suicide(creator)

def apply(account, action):
    if action == N('setgreeting'):
        greeter = Greeter()
#        print('original greeting:',greeter.greeting)
        greeter.greeting = read_action()

    elif action == N('greeting'):
        greeter = Greeter()
        print(greeter.greeting)


