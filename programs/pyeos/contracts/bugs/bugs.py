from eoslib import *
import ustruct as struct

class T(object):
    def __init__(self):
        #should be super(T, self).__init__()
        super(self, T).__init__()

def apply(name, type):
    if type == N('test'):
        t = T()
