# %load network.py

"""
network.py
~~~~~~~~~~
IT WORKS

A module to implement the stochastic gradient descent learning
algorithm for a feedforward neural network.  Gradients are calculated
using backpropagation.  Note that I have focused on making the code
simple, easily readable, and easily modifiable.  It is not optimized,
and omits many desirable features.
"""

#### Libraries
# Standard library
import random
import struct
import numpy as np
import zlib
import pickle

import eoslib
from eoslib import N

# Third-party libraries

mnist = N(b'mnist')
table_network = N(b'network')

class Network(object):

    def __init__(self, sizes):
        """The list ``sizes`` contains the number of neurons in the
        respective layers of the network.  For example, if the list
        was [2, 3, 1] then it would be a three-layer network, with the
        first layer containing 2 neurons, the second layer 3 neurons,
        and the third layer 1 neuron.  The biases and weights for the
        network are initialized randomly, using a Gaussian
        distribution with mean 0, and variance 1.  Note that the first
        layer is assumed to be an input layer, and by convention we
        won't set any biases for those neurons, since biases are only
        ever used in computing the outputs from later layers."""
        self.num_layers = len(sizes)
        self.sizes = sizes
        if not self.load():
            self.biases = [np.random.randn(y, 1) for y in sizes[1:]]
            self.weights = [np.random.randn(y, x)
                            for x, y in zip(sizes[:-1], sizes[1:])]

    def load(self):
        sizeweights = 0
        sizebiases = 0
    
        keys = struct.pack('Q', N('sizeweights'))
        values = bytes(8)
        if eoslib.load(mnist, mnist, table_network, keys, 0, 0, values) > 0:
            sizeweights = int.from_bytes(values, 'little')
        
        if sizeweights <= 0:
            return False
    
        keys = struct.pack('Q', N('sizebiases'))
        values = bytes(8)
        if eoslib.load(mnist, mnist, table_network, keys, 0, 0, values) > 0:
            sizebiases = int.from_bytes(values, 'little')
    
        if sizebiases <= 0:
            return False

        print("+++++++++++++++++", sizeweights, sizebiases)
        
        keys = struct.pack('Q', N('weights'))
        values = bytes(sizeweights)
        if eoslib.load(mnist, mnist, table_network, keys, 0, 0, values) <= 0:
            return False
        self.weights = pickle.loads(values)
    
        keys = struct.pack('Q', N('biases'))
        values = bytes(sizebiases)
        if eoslib.load(mnist, mnist, table_network, keys, 0, 0, values) <= 0:
            return False
        self.biases = pickle.loads(values)
        print('load success!')
        return True

    def save(self):
        biases = pickle.dumps(self.biases)
        weights = pickle.dumps(self.weights)

        keys = struct.pack('Q', N('sizeweights'))
        values = struct.pack('Q', len(weights))
        eoslib.store(mnist, table_network, keys, 0, values)

        keys = struct.pack('Q', N('sizebiases'))
        values = struct.pack('Q', len(biases))
        eoslib.store(mnist, table_network, keys, 0, values)

        keys = struct.pack('Q', N('weights'))
        eoslib.store(mnist, table_network, keys, 0, weights)

        keys = struct.pack('Q', N('biases'))
        eoslib.store(mnist, table_network, keys, 0, biases)

    def feedforward(self, a):
        """Return the output of the network if ``a`` is input."""
        for b, w in zip(self.biases, self.weights):
            a = sigmoid(np.dot(w, a) + b)
        return a

    def SGD(self, training_data, epochs, mini_batch_size, eta,
            test_data=None):
        """Train the neural network using mini-batch stochastic
        gradient descent.  The ``training_data`` is a list of tuples
        ``(x, y)`` representing the training inputs and the desired
        outputs.  The other non-optional parameters are
        self-explanatory.  If ``test_data`` is provided then the
        network will be evaluated against the test data after each
        epoch, and partial progress printed out.  This is useful for
        tracking progress, but slows things down substantially."""

        training_data = list(training_data)
        n = len(training_data)

        if test_data:
            test_data = list(test_data)
            n_test = len(test_data)

        for j in range(epochs):
            random.shuffle(training_data)
            mini_batches = [
                training_data[k:k + mini_batch_size]
                for k in range(0, n, mini_batch_size)]
            for mini_batch in mini_batches:
                self.update_mini_batch(mini_batch, eta)
            if test_data:
                print("Epoch {} : {} / {}".format(j, self.evaluate(test_data), n_test));
        self.save()
        
    def test(self, test_data):
        test_data = list(test_data)
        n_test = len(test_data)
        print("{} / {}".format(self.evaluate(test_data), n_test));

    def update_mini_batch(self, mini_batch, eta):
        """Update the network's weights and biases by applying
        gradient descent using backpropagation to a single mini batch.
        The ``mini_batch`` is a list of tuples ``(x, y)``, and ``eta``
        is the learning rate."""
        nabla_b = [np.zeros(b.shape) for b in self.biases]
        nabla_w = [np.zeros(w.shape) for w in self.weights]
        for x, y in mini_batch:
            delta_nabla_b, delta_nabla_w = self.backprop(x, y)
            nabla_b = [nb + dnb for nb, dnb in zip(nabla_b, delta_nabla_b)]
            nabla_w = [nw + dnw for nw, dnw in zip(nabla_w, delta_nabla_w)]
        self.weights = [w - (eta / len(mini_batch)) * nw
                        for w, nw in zip(self.weights, nabla_w)]
        self.biases = [b - (eta / len(mini_batch)) * nb
                       for b, nb in zip(self.biases, nabla_b)]

    def backprop(self, x, y):
        """Return a tuple ``(nabla_b, nabla_w)`` representing the
        gradient for the cost function C_x.  ``nabla_b`` and
        ``nabla_w`` are layer-by-layer lists of numpy arrays, similar
        to ``self.biases`` and ``self.weights``."""
        nabla_b = [np.zeros(b.shape) for b in self.biases]
        nabla_w = [np.zeros(w.shape) for w in self.weights]
        # feedforward
        activation = x
        activations = [x]  # list to store all the activations, layer by layer
        zs = []  # list to store all the z vectors, layer by layer
        for b, w in zip(self.biases, self.weights):
            z = np.dot(w, activation) + b
            zs.append(z)
            activation = sigmoid(z)
            activations.append(activation)
        # backward pass
        delta = self.cost_derivative(activations[-1], y) * \
            sigmoid_prime(zs[-1])
        nabla_b[-1] = delta
        nabla_w[-1] = np.dot(delta, activations[-2].transpose())
        # Note that the variable l in the loop below is used a little
        # differently to the notation in Chapter 2 of the book.  Here,
        # l = 1 means the last layer of neurons, l = 2 is the
        # second-last layer, and so on.  It's a renumbering of the
        # scheme in the book, used here to take advantage of the fact
        # that Python can use negative indices in lists.
        for l in range(2, self.num_layers):
            z = zs[-l]
            sp = sigmoid_prime(z)
            delta = np.dot(self.weights[-l + 1].transpose(), delta) * sp
            nabla_b[-l] = delta
            nabla_w[-l] = np.dot(delta, activations[-l - 1].transpose())
        return (nabla_b, nabla_w)

    def evaluate(self, test_data):
        """Return the number of test inputs for which the neural
        network outputs the correct result. Note that the neural
        network's output is assumed to be the index of whichever
        neuron in the final layer has the highest activation."""
        test_results = [(np.argmax(self.feedforward(x)), y)
                        for (x, y) in test_data]
        return sum(int(x == y) for (x, y) in test_results)

    def cost_derivative(self, output_activations, y):
        """Return the vector of partial derivatives \partial C_x /
        \partial a for the output activations."""
        return (output_activations - y)

#### Miscellaneous functions
def sigmoid(z):
    """The sigmoid function."""
    return 1.0 / (1.0 + np.exp(-z))

def sigmoid_prime(z):
    """Derivative of the sigmoid function."""
    return sigmoid(z) * (1 - sigmoid(z))

def vectorized_result(j):
    """Return a 10-dimensional unit vector with a 1.0 in the jth
    position and zeroes elsewhere.  This is used to convert a digit
    (0...9) into a corresponding desired output from the neural
    network."""
    e = np.zeros((10, 1))
    e[j] = 1.0
    return e

def train():
    eoslib.require_auth(eoslib.N(b'mnist'))
    data = eoslib.read_message()
    data = eoslib.unpack(data)
    print(len(data))
    data = zlib.decompress(data)
    data = pickle.loads(data)
    print('training start...')
#    print(data)
#    data0 = np.reshape(data[0], (784, 1))
#    data1 = vectorized_result(data[1])
    net = Network([784, 30, 10])
    net.SGD(data, 1, 1, 3.0, test_data=None)
    print('training end...')

def identify():
    n = np.argmax(self.feedforward(x))
    print(n)

def test():
    pass

def init():
    net = Network([784, 30, 10])
    return

def test():
    net = Network([784, 30, 10])
    biases = pickle.dumps(net.biases)
    weights = pickle.dumps(net.weights)
    print(len(weights))
    keys = struct.pack('Q', N('sizeweights'))
    values = struct.pack('Q', len(weights))
    ret = eoslib.store(mnist, table_network, keys, 0, values)
    print('store return:', ret)
    
    keys = struct.pack('Q', N('sizeweights'))
    values = bytes(8)
    eoslib.load(mnist, mnist, table_network, keys, 0, 0, values)
    print('load return:', int.from_bytes(values, 'little'))
    
    keys = struct.pack('Q', N('weights'))
    ret = eoslib.store(mnist, table_network, keys, 0, weights)
    print('store return:', ret)

    keys = struct.pack('Q', N('weights'))
    values = bytes(len(weights))
    ret = eoslib.load(mnist, mnist, table_network, keys, 0, 0, values)
    print('load return:', ret)
#    print(values)

#    print(net.weights)
#    print(net.biases)

def apply(code, action):
    print(code, action)
    if code == mnist:
        if action == eoslib.N(b'train'):
            train()
        elif action == eoslib.N(b'test'):
            pass


