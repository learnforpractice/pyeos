import os
import time
import sys
    
import pickle
import gzip
import numpy as np
import zlib

def init():
    import wallet
    import eosapi

    psw = 'PW5KTHfg4QA7wD1dZjbkpA97hEktDtQaip6hNNswWkmYo5pDK3CL1'
    wallet.open('mywallet')
    wallet.unlock('mywallet',psw)
    
    key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
    key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
    if not eosapi.get_account('mnist'):
        r = eosapi.create_account('inita', 'mnist',key1,key2)
        assert r
        num = eosapi.get_info().head_block_num
        while num == eosapi.get_info().head_block_num: # wait for finish of create account
            time.sleep(0.2)

    r = eosapi.set_contract('mnist','../../programs/pyeos/contract/mnist/mnist.py','../../programs/pyeos/contract/mnist/mnist.abi',1)
    assert r

'''
psw = 'PW5KTHfg4QA7wD1dZjbkpA97hEktDtQaip6hNNswWkmYo5pDK3CL1'
wallet.open('mywallet')
wallet.unlock('mywallet',psw)


key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
eosapi.create_account('inita', 'mnist',key1,key2)
import os
p = os.path.join(os.getcwd(),'../../programs/pyeos/contract/mnist/mnist.py')
r = eosapi.set_contract('mnist',p,'../../programs/pyeos/contract/mnist/mnist.abi',1)

'''

def vectorized_result(j):
    """Return a 10-dimensional unit vector with a 1.0 in the jth
    position and zeroes elsewhere.  This is used to convert a digit
    (0...9) into a corresponding desired output from the neural
    network."""
    e = np.zeros((10, 1))
    e[j] = 1.0
    return e    
    
def test():
    import eosapi
    '''
    p = os.path.join(os.getcwd(),'../../programs/pyeos/contract/mnist/mnist.pkl.gz')
    f = gzip.open(p, 'rb')
    training_data, validation_data, test_data = pickle.load(f, encoding="latin1")
    f.close()
    print('load done!')
    training_data = list(training_data)
    '''
    p = os.path.join(os.getcwd(),'../../programs/pyeos/contract/mnist')
    sys.path.insert(0,p)
    import mnist_loader
    training_data, validation_data, test_data = mnist_loader.load_data_wrapper()
    training_data = list(training_data)
    for d in training_data[:100]:
        data = pickle.dumps([d,])
        data = zlib.compress(data)
    #    print(data)
        r = eosapi.push_message('mnist','train',data,['mnist'],{'mnist':'active'},rawargs=True)

'''

r = eosapi.set_contract('mnist','../../programs/pyeos/contract/mnist/mnist.py','../../programs/pyeos/contract/mnist/mnist.abi',1)

import binascii
data = bytes(128)
data = binascii.hexlify(data)
r = eosapi.push_message('mnist','train',data,['mnist'],{'mnist':'active'},rawargs=True)

data = bytes(128)
r = eosapi.push_message('mnist','train',{'data':bytes(10)},['mnist'],{'mnist':'active'})

'''        
if __name__ == '__main__':
    sys.path.insert(0,'..')
    import mnist

    net = mnist.Network([784, 30, 10])

    import mnist_loader
    training_data, validation_data, test_data = mnist_loader.load_data_wrapper()
    training_data = list(training_data)


#    print(data)
#    data0 = np.reshape(data[0], (784, 1))
#    data1 = vectorized_result(data[1])
    
    net.SGD(training_data[:1], 1, 1, 3.0, test_data=None)



