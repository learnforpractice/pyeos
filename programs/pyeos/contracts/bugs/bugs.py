import ustruct as struct
from eoslib import *
from backyard.solidity import payable

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
        print('+++++++++++ ++ RuntimeError')

class List(list):
    def __new__(cls, value):
        print('call new')
        return  super(List, cls).__new__(cls, value)

@payable
def p1():
    pass

@payable
def p2():
    p1()

def apply(name, type):
    if type == N('t1'):
        try:
            t = T()
        except RuntimeError:
            print('+++++++++++++RuntimeError')
        test_recursive_gen()
    elif type == N('t2'):
        p2()
    elif type == N('t4'):
        try:
            l = List([1,3,3])
        except AttributeError as e:
            print(e) 

