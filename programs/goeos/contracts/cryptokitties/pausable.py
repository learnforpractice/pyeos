from backend import *
from basement import *
#from ownable import Ownable
from ownable import *

def whenNotPaused(func):
    def func_wrapper(self, *args):
        require(not self.paused)
        return func(self, *args)
    return func_wrapper

'''
* @dev modifier to allow actions only when the contract IS NOT paused
'''
def whenPaused(func):
    def func_wrapper(self, *args):
        require(self.paused)
        return func(self, *args)
    return func_wrapper

   
'''
 * @title Pausable
 * @dev Base contract which allows children to implement an emergency stop mechanism.
'''
 
class Pausable(Ownable):
    '''FIXME: event
      event Pause();
      event Unpause();
    '''

    def __init__(self):
        super(Pausable,self).__init__()
        self.paused = False


    '''
    * @dev modifier to allow actions only when the contract IS paused
    FIXME: modifier
    modifier whenNotPaused() {
      require(!paused);
      _;
    }
    '''

    '''
   * @dev called by the owner to pause, triggers stopped state
    '''
    @onlyOwner
    @whenNotPaused
    def pause(self) -> bool:
        self.paused = True;
        self.Pause();
        return True;

    '''
   * @dev called by the owner to unpause, returns to normal state
    '''
    @onlyOwner
    @whenNotPaused
    def unpause(self) -> bool:
        self.paused = False
        self.Unpause()
        return True
