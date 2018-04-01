#KittyAccessControl

#/ @dev Access modifier for CEO-only functionality
def onlyCEO(func):
   def func_wrapper(self, *args):
       require_auth(self._ceoAddress)
       return func(self, *args)
   return func_wrapper

#/ @dev Access modifier for CFO-only functionality
def onlyCFO(func):
   def func_wrapper(self, *args):
       require_auth(self._cfoAddress)
       return func(self, *args)
   return func_wrapper

#/ @dev Access modifier for COO-only functionality
def onlyCOO(func):
   def func_wrapper(self, *args):
       require_auth(self._cooAddress)
       return func(self, *args)
   return func_wrapper

def onlyCLevel(func):
   def func_wrapper(self, *args):
       eosio_assert(msg.sender == cooAddress or msg.sender == ceoAddress or msg.sender == cfoAddress, "only clevel")
       return func(self, *args)
   return func_wrapper

#/ @dev Modifier to allow actions only when the contract IS NOT paused
def whenNotPaused(func):
   def func_wrapper(self, *args):
       require(not self._paused)
       return func(self, *args)
   return func_wrapper

#/ @dev Modifier to allow actions only when the contract IS paused
def whenPaused(func):
   def func_wrapper(self, *args):
       require(self._paused)
       return func(self, *args)
   return func_wrapper

#Ownable
def onlyOwner(func):
   def func_wrapper(self, *args):
       require_auth(self.owner)
       return func(self, *args)
   return func_wrapper

def event(func):
   def func_wrapper(self, *args):
       print('TODO: event')
       return func(self, *args)
   return func_wrapper
