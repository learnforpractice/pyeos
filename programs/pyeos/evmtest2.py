import json
import web3

from web3 import Web3, HTTPProvider, TestRPCProvider, EthereumTesterProvider
from solc import compile_source
from web3.contract import ConciseContract
import eosapi

class Producer(object):
    def __init__(self, provider):
        self.provider = provider
    
    def produce_block(self):
        pass
#        self.provider.make_request('test_mineBlocks', [1])

    def __call__(self):
        eosapi.produce_block()

    def __enter__(self):
        pass

    def __exit__(self, type, value, traceback):
        self.produce_block()

contract_source_code = '''
pragma solidity ^0.4.0;
contract Greeter {
    mapping(address => uint) public voters;

    function Greeter() {
        voters[msg.sender] = 31415926;
    }

    function getHash(uint blockNumber) public returns (bytes32) {
        block.difficulty;
        sha256('+++++++++++++++++++hello, smart contract abcdef');
        voters[msg.sender] = blockNumber;
        return block.blockhash(blockNumber);
    }

    function getValue() public returns (uint) {
        block.difficulty;
        sha256('+++++++++++++++++++hello, smart contract abcdef');
        return voters[msg.sender];
    }

    function setValue(uint v) public {
        voters[msg.sender] = v;
        v = voters[msg.sender];
    }

    function getDiff() public returns (uint) {
        return block.difficulty;
    }
    
}

'''


from eth_utils import (
    to_dict,
)

class LocalProvider(web3.providers.base.JSONBaseProvider):
    endpoint_uri = None
    _request_args = None
    _request_kwargs = None

    def __init__(self, request_kwargs=None):
        self._request_kwargs = request_kwargs or {}
        super(LocalProvider, self).__init__()

    def __str__(self):
        return "RPC connection {0}".format(self.endpoint_uri)

    @to_dict
    def get_request_kwargs(self):
        if 'headers' not in self._request_kwargs:
            yield 'headers', self.get_request_headers()
        for key, value in self._request_kwargs.items():
            yield key, value

    def request_func_(self, method, params):
        print('+++++++request_func_:', method, params)
        if method == 'eth_sendTransaction':
            if 'to' in params[0]:
                r = eosapi.push_evm_message(params[0]['to'], params[0]['data'], {params[0]['from']:'active'}, True,rawargs=True)
            else:
                r = eosapi.set_evm_contract(params[0]['from'], params[0]['data'])
            if r:
                return {'result':r}
        elif method == 'eth_call':
            return {"id":0,"jsonrpc":"2.0","result":123}
        elif method == 'eth_estimateGas':
            return {"id":0,"jsonrpc":"2.0","result":88}
        elif method == 'eth_blockNumber':
            return {"id":0,"jsonrpc":"2.0","result":15}
        elif method == 'eth_getBlock':
            result = {'author': '0x4b8823fda79d1898bd820a4765a94535d90babf3', 'extraData': '0xdc809a312e332e302b2b313436372a4444617277692f6170702f496e74', 'gasLimit': 3141592, 'gasUsed': 0, 'hash': '0x259d3ac184c567e4e3aa3fb0aa6c89d39dd172f6dad2c7e26265b40dce2f8893', 'logsBloom': '0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000', 'miner': '0x4b8823fda79d1898bd820a4765a94535d90babf3', 'number': 138, 'parentHash': '0x7ed0cdae409d5b785ea671e24408ab34b25cb450766e501099ad3050afeff71a', 'receiptsRoot': '0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421', 'sha3Uncles': '0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347', 'stateRoot': '0x1a0789d0d895011034cda1007a4be75faee0b91093c784ebf246c8651dbf699b', 'timestamp': 1521704325, 'totalDifficulty': 131210, 'transactions': [], 'transactionsRoot': '0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421', 'uncles': []}
            return {"id":0,"jsonrpc":"2.0","result":result}
        elif method == 'eth_getBlockByNumber':
            result = {'author': '0x4b8823fda79d1898bd820a4765a94535d90babf3', 'extraData': '0xdc809a312e332e302b2b313436372a4444617277692f6170702f496e74', 'gasLimit': 3141592, 'gasUsed': 0, 'hash': '0x259d3ac184c567e4e3aa3fb0aa6c89d39dd172f6dad2c7e26265b40dce2f8893', 'logsBloom': '0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000', 'miner': '0x4b8823fda79d1898bd820a4765a94535d90babf3', 'number': 138, 'parentHash': '0x7ed0cdae409d5b785ea671e24408ab34b25cb450766e501099ad3050afeff71a', 'receiptsRoot': '0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421', 'sha3Uncles': '0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347', 'stateRoot': '0x1a0789d0d895011034cda1007a4be75faee0b91093c784ebf246c8651dbf699b', 'timestamp': 1521704325, 'totalDifficulty': 131210, 'transactions': [], 'transactionsRoot': '0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421', 'uncles': []}
            return {"id":0,"jsonrpc":"2.0","result":result}
        elif method == 'eth_blockNumber':
            return {"id":0,"jsonrpc":"2.0","result":'100'}

    def request_func(self, web3, outer_middlewares):
        print( web3, outer_middlewares)
        '''
        @param outer_middlewares is an iterable of middlewares, ordered by first to execute
        @returns a function that calls all the middleware and eventually self.make_request()
        '''
        return self.request_func_
    
    def get_request_headers(self):
        return {
            'Content-Type': 'application/json',
            'User-Agent': construct_user_agent(str(type(self))),
        }

    def make_request(self, method, params):
        print('+++++++++++++:make_request', method, params)
        if method == 'eth_sendTransaction':
            if 'to' in params[0]:
                r = eosapi.push_message(params[0]['to'], '', args, {params[0]['from']:'active'}, True,rawargs=True)
            else:
                r = eosapi.set_evm_contract(params[0]['from'], params[0]['data'])
            if r:
                return {'result':str(r)}
#{"id":0,"jsonrpc":"2.0","result":["0xc2ff44dd289190eb47839a3e7bab1ee1abe1ebbe"]}
#raw_response    bytes: b'{"id":3,"jsonrpc":"2.0","result":{"author":"0x4b8823fda79d1898bd820a4765a94535d90babf3","extraData":"0xdc809a312e332e302b2b313436372a4444617277692f6170702f496e74","gasLimit":"0x2fefd8","gasUsed":"0x0","hash":"0x259d3ac184c567e4e3aa3fb0aa6c89d39dd172f6dad2c7e26265b40dce2f8893","logsBloom":"0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000","miner":"0x4b8823fda79d1898bd820a4765a94535d90babf3","number":"0x8a","parentHash":"0x7ed0cdae409d5b785ea671e24408ab34b25cb450766e501099ad3050afeff71a","receiptsRoot":"0x56e81f171bcc55a6...    
#{'author': '0x4b8823fda79d1898bd820a4765a94535d90babf3', 'extraData': '0xdc809a312e332e302b2b313436372a4444617277692f6170702f496e74', 'gasLimit': 3141592, 'gasUsed': 0, 'hash': '0x259d3ac184c567e4e3aa3fb0aa6c89d39dd172f6dad2c7e26265b40dce2f8893', 'logsBloom': '0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000', 'miner': '0x4b8823fda79d1898bd820a4765a94535d90babf3', 'number': 138, 'parentHash': '0x7ed0cdae409d5b785ea671e24408ab34b25cb450766e501099ad3050afeff71a', 'receiptsRoot': '0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421', 'sha3Uncles': '0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347', 'stateRoot': '0x1a0789d0d895011034cda1007a4be75faee0b91093c784ebf246c8651dbf699b', 'timestamp': 1521704325, 'totalDifficulty': 131210, 'transactions': [], 'transactionsRoot': '0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421', 'uncles': []}



TEST = False
DEPLOY = True

contract_interface = None
w3 = None
producer = None
contract = None
contract_address = None

provider = LocalProvider()
producer = Producer(provider)

if TEST:
    w3 = Web3(EthereumTesterProvider())
else:
    w3 = Web3(provider)

#"<stdin>:Greeter"
def compile(main_class):
    global contract_interface
    global producer
    global w3
    global contract
    
    compiled_sol = compile_source(contract_source_code) # Compiled source code
    
    for key in compiled_sol.keys():
        print(key)
        
    s = json.dumps(compiled_sol[main_class], sort_keys=False, indent=4, separators=(',', ': '))
#    print(s)
    
    contract_interface = compiled_sol[main_class]
    #print(contract_interface)
    
    # web3.py instance
    #w3 = Web3(TestRPCProvider())
    #w3 = Web3(HTTPProvider())
    
    # Instantiate and deploy contract
    contract = w3.eth.contract(contract_interface['abi'], bytecode=contract_interface['bin'])
    #print(contract_interface['abi'])
    print(contract_interface['bin'])
    with open('/Users/newworld/dev/pyeos/build/programs/mtn.bin', 'w') as f:
        f.write(contract_interface['bin'])

    json.dumps(contract_interface['abi'], sort_keys=False, indent=4, separators=(',', ': '))


def deploy():
    global contract_address
    with producer:
        # Get transaction hash from deployed contract
        address = eosapi.eos_name_to_eth_address('evm') #w3.eth.accounts[0]
        with producer:
            tx_hash = contract.deploy(transaction={'from': address, 'gas': 2000001350})
#            tx_hash = contract.deploy(transaction={'from': address})
            print('tx_hash:', tx_hash)

#    contract_address = '0x5c5ad149D975c0f6EcB9F19BA2F917AFdCD0AA41'

#0x24bC35FA6f3f81EFD2c7F3ba9862470B805982D3
#0x2445185DDc617d3240d4F0FdA365466CF3CF39F6

contract_instance = None
#personal.unlockAccount(eth.accounts[0],'abc')
'''
args = '0x6b2fafa9000000000000000000000000000000000000000000000000000000000001d0d8'
code = '6060604052341561000f57600080fd5b600260006040516020015260405180807f6162630000000000000000000000000000000000000000000000000000000000815250600301905060206040518083038160008661646e5a03f1151561006557600080fd5b5050604051805190505060405180807f6162630000000000000000000000000000000000000000000000000000000000815250600301905060405180910390505060806000803373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002081905550610231806100f96000396000f300606060405260043610610057576000357c0100000000000000000000000000000000000000000000000000000000900463ffffffff1680630b1bcee71461005c5780636b2fafa914610085578063a3ec138d146100c4575b600080fd5b341561006757600080fd5b61006f610111565b6040518082815260200191505060405180910390f35b341561009057600080fd5b6100a66004808035906020019091905050610119565b60405180826000191660001916815260200191505060405180910390f35b34156100cf57600080fd5b6100fb600480803573ffffffffffffffffffffffffffffffffffffffff169060200190919050506101ed565b6040518082815260200191505060405180910390f35b600044905090565b6000600260006040516020015260405180807f2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b68656c6c6f2c20736d6172742081526020017f636f6e7472616374206162636465660000000000000000000000000000000000815250602f01905060206040518083038160008661646e5a03f1151561019757600080fd5b50506040518051905050816000803373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208190555081409050919050565b600060205280600052604060002060009150905054815600a165627a7a72305820a170f59df0c8bfb1ce2d223eaabe4cdbb54a4102cf2678d40692e3b03ac8f73f0029'
debug.evm_test(code, args) 

code = '6060604052341561000f57600080fd5b600260006040516020015260405180807f6162630000000000000000000000000000000000000000000000000000000000815250600301905060206040518083038160008661646e5a03f1151561006557600080fd5b5050604051805190505060405180807f6162630000000000000000000000000000000000000000000000000000000000815250600301905060405180910390505060806000803373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002081905550610231806100f96000396000f300606060405260043610610057576000357c0100000000000000000000000000000000000000000000000000000000900463ffffffff1680630b1bcee71461005c5780636b2fafa914610085578063a3ec138d146100c4575b600080fd5b341561006757600080fd5b61006f610111565b6040518082815260200191505060405180910390f35b341561009057600080fd5b6100a66004808035906020019091905050610119565b60405180826000191660001916815260200191505060405180910390f35b34156100cf57600080fd5b6100fb600480803573ffffffffffffffffffffffffffffffffffffffff169060200190919050506101ed565b6040518082815260200191505060405180910390f35b600044905090565b6000600260006040516020015260405180807f2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b68656c6c6f2c20736d6172742081526020017f636f6e7472616374206162636465660000000000000000000000000000000000815250602f01905060206040518083038160008661646e5a03f1151561019757600080fd5b50506040518051905050816000803373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208190555081409050919050565b600060205280600052604060002060009150905054815600a165627a7a72305820a170f59df0c8bfb1ce2d223eaabe4cdbb54a4102cf2678d40692e3b03ac8f73f0029'
debug.evm_test('', code) 

args = '0x6b2fafa9000000000000000000000000000000000000000000000000000000000001d0d8'
code = '606060405260043610610057576000357c0100000000000000000000000000000000000000000000000000000000900463ffffffff1680630b1bcee71461005c5780636b2fafa914610085578063a3ec138d146100c4575b600080fd5b341561006757600080fd5b61006f610111565b6040518082815260200191505060405180910390f35b341561009057600080fd5b6100a66004808035906020019091905050610119565b60405180826000191660001916815260200191505060405180910390f35b34156100cf57600080fd5b6100fb600480803573ffffffffffffffffffffffffffffffffffffffff169060200190919050506101ed565b6040518082815260200191505060405180910390f35b600044905090565b6000600260006040516020015260405180807f2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b68656c6c6f2c20736d6172742081526020017f636f6e7472616374206162636465660000000000000000000000000000000000815250602f01905060206040518083038160008661646e5a03f1151561019757600080fd5b50506040518051905050816000803373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208190555081409050919050565b600060205280600052604060002060009150905054815600a165627a7a72305820a170f59df0c8bfb1ce2d223eaabe4cdbb54a4102cf2678d40692e3b03ac8f73f0029'
debug.evm_test(code, args) 

'''

def set_greeting():
    global contract_instance
    address = 'evm'
    
    contract_address = eosapi.eos_name_to_eth_address('evm') #'0x2445185DDc617d3240d4F0FdA365466CF3CF39F6' #'evm'

    # Contract instance in concise mode
    contract_instance = w3.eth.contract(contract_interface['abi'], contract_address, ContractFactoryClass=ConciseContract)
#    print(dir(contract_instance))

#    r = contract_instance.getDiff(transact={'from': address})
#    print(r)
    print(contract_instance)
    with producer:
        r = contract_instance.setValue(119000, transact={'from': address})
        print('++++++++++++setValue:', r)
    
    #    r = contract_instance.getValue(transact={'from': address})
    #    r = contract_instance.getValue(call={'from': address})
    #    r = contract_instance.getValue(call={})
        r = contract_instance.getValue(transact={'from': address})
    
        print('++++++++++getValue:', r)

#    r = contract_instance.getHash(119000, transact={'from': address})
#    print(r)
    
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

name = "<stdin>:Greeter"
#name = "<stdin>:MotionToken"
compile(name)

deploy()
set_greeting()




