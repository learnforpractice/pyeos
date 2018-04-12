from eoslib import *
import ustruct as struct

class T(object):
    def __init__(self):
        #should be super(T, self).__init__()
        super(self, T).__init__()

# test deeply recursive generators
def gen():
    yield from gen()

def test_recursive_gen():
    try:
        #should not cause segment fault
        list(gen())
    except RuntimeError:
        print('+++++++++++++RuntimeError')

class List(list):
    def __new__(cls, value):
        return  super(List, cls).__new__(cls, value)

def apply(name, type):
    if type == N('t1'):
        try:
            t = T()
        except RuntimeError:
            print('+++++++++++++RuntimeError')
        test_recursive_gen()
        l = List([1,3,3])
