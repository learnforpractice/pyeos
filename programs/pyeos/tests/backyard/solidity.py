from eoslib import eosio_assert

def event(func):
    def func_wrapper(self, *args):
        print('TODO: event')
        return func(self, *args)
    return func_wrapper

#FIXME it works, yet not safe since payable.count can be changed deliberately
#It'd better to implement payable decorator in C
class payable(object):
    count = 0
    def __init__(self, func):
        self.func = func
        payable.count = 0

    def __call__(self, *args):
        assert payable.count == 0, 'reentrant in payable function is forbidden!'
        payable.count = 1
        ret = self.func(*args)
        payable.count = 0
        return ret

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
