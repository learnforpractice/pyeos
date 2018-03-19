import json
import web3

from web3 import Web3, HTTPProvider, TestRPCProvider, EthereumTesterProvider
from solc import compile_source
from web3.contract import ConciseContract

class Producer(object):
    def __init__(self, provider):
        self.provider = provider
    
    def produce_block(self):
        self.provider.make_request('test_mineBlocks', [1])

    def __call__(self):
        self.produce_block()

    def __enter__(self):
        pass
    
    def __exit__(self, type, value, traceback):
        self.produce_block()

contract_source_code = '''
pragma solidity ^0.4.0;
contract Greeter {
    function Greeter() {
        sha256('abc');
        keccak256('abc');
    }

    function getHash(uint blockNumber) public returns (bytes32) {
        block.difficulty;
        sha256('+++++++++++++++++++hello, smart contract abcdef');
        return block.blockhash(blockNumber);
    }
    
    function getDiff() public returns (uint) {
        return block.difficulty;
    }
    
}

'''

TEST = False
DEPLOY = True

contract_interface = None
w3 = None
producer = None
contract = None
contract_address = None

provider = HTTPProvider('http://localhost:8545')
producer = Producer(provider)

if TEST:
    w3 = Web3(EthereumTesterProvider())
else:
    w3 = Web3(provider)


def compile():
    global contract_interface
    global producer
    global w3
    global contract
    
    compiled_sol = compile_source(contract_source_code) # Compiled source code
    s = json.dumps(compiled_sol["<stdin>:Greeter"], sort_keys=False, indent=4, separators=(',', ': '))
#    print(s)
    
    contract_interface = compiled_sol['<stdin>:Greeter']
    #print(contract_interface)
    
    # web3.py instance
    #w3 = Web3(TestRPCProvider())
    #w3 = Web3(HTTPProvider())
    
    
    address = w3.eth.accounts[0]
    
    # Instantiate and deploy contract
    contract = w3.eth.contract(contract_interface['abi'], bytecode=contract_interface['bin'])
    #print(contract_interface['abi'])
    print(contract_interface['bin'])
    
    json.dumps(contract_interface['abi'], sort_keys=False, indent=4, separators=(',', ': '))
    
    print('+++++++++w3.eth.accounts[0]:', address)
    print('+++++++++w3.eth.blockNumber:', w3.eth.blockNumber)

def deploy():
    global contract_address
    with producer:
        # Get transaction hash from deployed contract
        address = w3.eth.accounts[0]
        print('----------w3.eth.blockNumber:', w3.eth.blockNumber)
        with producer:
            tx_hash = contract.deploy(transaction={'from': address, 'gas': 2000001350})
#            tx_hash = contract.deploy(transaction={'from': address})
            print('tx_hash:', tx_hash)
        print('----------w3.eth.blockNumber:', w3.eth.blockNumber)

        # Get tx receipt to get contract address
        tx_receipt = w3.eth.getTransactionReceipt(tx_hash)
##        print('tx_receipt:', tx_receipt)
        contract_address = tx_receipt['contractAddress']
        
        print('-------------------------:', tx_receipt.contractAddress)

##        print(tx_receipt)
        print('contract_address:', contract_address)

#    contract_address = '0x5c5ad149D975c0f6EcB9F19BA2F917AFdCD0AA41'

#0x24bC35FA6f3f81EFD2c7F3ba9862470B805982D3
#0x2445185DDc617d3240d4F0FdA365466CF3CF39F6

contract_instance = None

def set_greeting():
    global contract_instance
    print('contract_address:', contract_address)
    address = w3.eth.accounts[0]

    # Contract instance in concise mode
    contract_instance = w3.eth.contract(contract_interface['abi'], contract_address, ContractFactoryClass=ConciseContract)
#    print(dir(contract_instance))

    r = contract_instance.getDiff(transact={'from': address})
    print(r)

    r = contract_instance.getHash(10, transact={'from': address})
    print(r)
    
    if 0:
        with producer:
            contract_instance.setGreeting('Nihao', transact={'from': address})
    
        if 0:
            # Getters + Setters for web3.eth.contract object
            print('Contract value: {}'.format(contract_instance.greet()))
            print('+++++++++w3.eth.blockNumber:', w3.eth.blockNumber)
        
            with producer:
                contract_instance.setGreeting('Nihao', transact={'from': address})
            print('Setting value to: Nihao')
            print('Contract value: {}'.format(contract_instance.greet()))
        
            print('Contract value: {}'.format(contract_instance.helloword()))
            print('Contract getint: {}'.format(contract_instance.getint()))
        
        r = contract_instance.getHash(10).encode('utf8')
        print(len(r))
        
        print('Contract getHash: {}'.format(r))
        
        if 0:
            print('Contract value: {}'.format(contract_instance.getAddress()))

compile()
deploy()
set_greeting()




