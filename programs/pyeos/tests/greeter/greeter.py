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
from eoslib import N, read_action
import ustruct as struct
from backyard.storage import storage_set, storage_get, storage_find

code = N('greeter')


def storage(func):
    def func_wrapper(self, *args):
        return func(self, *args)
    return func_wrapper

class Greeter:
    def __init__(self, _greeting = None):
#        self.creator = msg.sender
        self._greeting = storage_find(code, N('mytable'), N('greeting'))
        if self._greeting is None:
            self._greeting = ''

    @property
    def greeting(self):
        return self._greeting

    @greeting.setter
    def greeting(self, value):
        self._greeting = value
        storage_set(code, N('mytable'), N('greeting'), value.encode('utf8'))

    def kill(self):
        pass
#FIXME
#        if msg.sender == self.creator:
#            suicide(creator)

def apply(receiver, code, action):
    if action == N('setgreeting'):
        greeter = Greeter()
#        print('original greeting:',greeter.greeting)
        value = read_action()
        greeter.greeting = value.decode('utf8')
    elif action == N('greeting'):
        greeter = Greeter()
        print(greeter.greeting)

