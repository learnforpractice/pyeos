from backend import *

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

