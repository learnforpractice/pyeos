import os
import time
import sys
    
import pickle
import gzip
#import numpy as np
import zlib
try:
    import eosapi
    producer = eosapi.Producer()
    print('This DL example is not supported anymore, a turly AI on blockchain will not looks like this.')
    print('Please make sure you are running the following command before test')
    print('./pyeos/pyeos --manual-gen-block --debug -i')
except Exception as e:
    print(e)

def init():
    '''
    psw = 'PW5Kd5tv4var9XCzvQWHZVyBMPjHEXwMjH1V19X67kixwxRpPNM4J'
    wallet.open('mywallet')
    wallet.unlock('mywallet',psw)
    '''
    
    key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
    key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
    if not eosapi.get_account('mnist'):
        with producer:
            r = eosapi.create_account('inita', 'mnist', key1, key2)
            assert r
    with producer:
        r = eosapi.set_contract('mnist', '../../programs/pyeos/contracts/mnist/mnist.py', '../../programs/pyeos/contracts/mnist/mnist.abi', eosapi.py_vm_type)
        assert r

def vectorized_result(j):
    """Return a 10-dimensional unit vector with a 1.0 in the jth
    position and zeroes elsewhere.  This is used to convert a digit
    (0...9) into a corresponding desired output from the neural
    network."""
    e = np.zeros((10, 1))
    e[j] = 1.0
    return e    
    
def test():
    producer()
    p = os.path.join(os.getcwd(), '../../programs/pyeos/contracts/mnist')
    sys.path.insert(0, p)
    import mnist_loader
    training_data, validation_data, test_data = mnist_loader.load_data_wrapper()
    print('load done!')
    training_data = list(training_data)
    txids = []
    counter = 0
    for d in training_data[:1]:
        data = pickle.dumps([d, ])
        data = zlib.compress(data)
    #    print(data)
        r = eosapi.push_message('mnist', 'train', data, ['mnist'], {'mnist':'active'}, rawargs=True)
        assert r
        print(r.transaction_id)
        if r.transaction_id in txids:
            raise 'dumplicate ts id'
        txids.append(r.transaction_id)
        counter += 1
        if counter % 50 == 0:
            print(counter)
            with producer:
                pass
    producer()

if __name__ == '__main__':
    sys.path.insert(0, '..')
    import mnist

    net = mnist.Network([784, 30, 10])

    import mnist_loader
    training_data, validation_data, test_data = mnist_loader.load_data_wrapper()
    training_data = list(training_data)


#    print(data)
#    data0 = np.reshape(data[0], (784, 1))
#    data1 = vectorized_result(data[1])
    
    net.SGD(training_data[:1], 1, 1, 3.0, test_data=None)



