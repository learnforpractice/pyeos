import os
import json
import web3

from web3 import Web3, HTTPProvider, TestRPCProvider, EthereumTesterProvider
from solc import compile_source
from web3.contract import ConciseContract


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
        print('----request_func', method, params)
        if method == 'eth_sendTransaction':
            print(params)
            if 'to' in params[0]:
                r, cost = eosapi.push_evm_action(params[0]['to'], params[0]['data'], {params[0]['from']:'active'}, True)
            else:
                r, cost  = eosapi.set_evm_contract(params[0]['from'], params[0]['data'])
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



TEST = False
DEPLOY = True

contract_interface = None

provider = LocalProvider()

if TEST:
    w3 = Web3(EthereumTesterProvider())
else:
    w3 = Web3(provider)


contract = None
contract_address = None


def compile(contract_source_code, main_class):

    compiled_sol = compile_source(contract_source_code) # Compiled source code
#    print(compiled_sol)
    
#    s = json.dumps(compiled_sol[main_class], sort_keys=False, indent=4, separators=(',', ': '))
    for _class in compiled_sol.keys():
        print(_class)

    contract_interface = compiled_sol[main_class]
    return contract_interface

def deploy(contract_interface):
    contract = w3.eth.contract(contract_interface['abi'], bytecode=contract_interface['bin'])
#    print(contract_interface['bin'])
    json.dumps(contract_interface['abi'], sort_keys=False, indent=4, separators=(',', ': '))

    address = eosapi.eos_name_to_eth_address('evm')
    tx_hash = contract.deploy(transaction={'from': address, 'gas': 2000001350})
    print('tx_hash:', tx_hash)
    print('=========================deploy end======================')

def call_contract(contract_interface):
    print(contract_interface['abi'])
    contract_address = '0x88224fe4e4954c29d1d765ebe39331186fc259cf'
    contract_address = Web3.toChecksumAddress(contract_address)
    
    contract_instance = w3.eth.contract(contract_address, abi=contract_interface['abi'], bytecode=contract_interface['bin'], ContractFactoryClass=ConciseContract)

    # Contract instance in concise mode
#    contract_instance = w3.eth.contract(contract_interface['abi'], contract_address, ContractFactoryClass=ConciseContract)


#    r = contract_instance.getValue(transact={'from': address})
#    r = contract_instance.getValue(call={'from': address})
    if 0:
        r = contract_instance.getValue(transact={'from': contract_address})
        print('++++++++++getValue:', r)

    r = contract_instance.setValue(119000, transact={'from': contract_address})
    print('++++++++++++setValue:', r)

#    r = contract_instance.getValue(transact={'from': address})
#    r = contract_instance.getValue(call={'from': address})
    if 0:
        r = contract_instance.getValue(transact={'from': contract_address})
        print('++++++++++getValue:', r)

def test():
    main_class = '<stdin>:Greeter'
    greeter = os.path.join(os.path.dirname(__file__), 'greeter.sol')
    with open(greeter, 'r') as f:
        contract_source_code = f.read()
        contract_interface = compile(contract_source_code, main_class)
        call_contract(contract_interface)

def pack_args():

    fn_identifier = 'setValue'
    fn_abi =  {'constant': False, 'inputs': [{'name': 'v', 'type': 'uint256'}], 'name': 'setValue', 'outputs': [], 'payable': False, 'stateMutability': 'nonpayable', 'type': 'function'}
    args = (119000,)
    kwargs = {}
    
    contract_abi =  [
      {'constant': False, 'inputs': [],                                 'name': 'getValue', 'outputs': [{'name': '', 'type': 'uint256'}],      'payable': False, 'stateMutability': 'nonpayable', 'type': 'function'}, 
      {'constant': False, 'inputs': [{'name': 'v', 'type': 'uint256'}], 'name': 'setValue', 'outputs': [],                                     'payable': False, 'stateMutability': 'nonpayable', 'type': 'function'}, 
      {'constant': True, 'inputs': [{'name': '', 'type': 'address'}],   'name': 'mymap',    'outputs': [{'name': '',  'type': 'uint256'}],     'payable': False, 'stateMutability': 'view',       'type': 'function'}, 
      {'inputs': [], 'payable': False,  'stateMutability': 'nonpayable', 'type': 'constructor'}, 
      {'anonymous': False, 'inputs': [{'indexed': False, 'name': 's', 'type': 'string'}], 'name': 'log', 'type': 'event'}
      ]
    
    data = web3.utils.contracts.encode_transaction_data(
            web3,
            fn_identifier,
            contract_abi,
            fn_abi,
            args,
            kwargs)
    print(data)
test()

