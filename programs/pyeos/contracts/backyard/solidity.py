from eoslib import eosio_assert

def event(func):
    def func_wrapper(self, *args):
        print('TODO: event')
        return func(self, *args)
    return func_wrapper

#FIXME
def payable(func):
    def func_wrapper(self, *args):
        print('TODO: payable')
        return func(self, *args)
    return func_wrapper

class address(int):
    pass

def require(cond, msg = ''):
    if cond:
        cond = 1
    else:
        cond = 0
    eosio_assert(cond, msg)

class Msg(object):
    def __init__(self):
        self.sender = address(0)
        self.value = None
