A Self Evolving Universal Smart Contract Platform Base on The Development of EOSIO 
# Table of contents
1. [Building PyEos](#buildingpyeos)
2. [Creating Your First Python Smart Contract](#creatsmartcontract)
3. [Debugging With C++ Smart Contract](#smartcontractdebuggingcplusplus)
4. [Debugging With Python Smart Contract](#smartcontractdebuggingpython)
5. [PyEos api overview](#pyeosapioverview)

<a name="buildingPyEos"></a>

# Building PyEos

## Downloading Source Code

```bash
git clone https://www.github.com/learnforpractice/pyeos
cd pyeos
git submodule update --init --recursive
```

## Installing dependencies (Ubuntu)

```
sudo apt-get install libleveldb-dev
sudo apt-get install libreadline-dev
```

## Installing dependencies (macOS)

```
brew install leveldb
brew install readline
```

## Building

```bash
./eosio_build.sh
```

<a name="creatsmartcontract"></a>

# Creating Your First Python Smart Contract

### Running PyEos

Open a terminal, cd to [PROJECT_DIR]/build/program, run the following command

```
./pyeos/pyeos --manual-gen-block --debug -i  --contracts-console
```

If it's the first time you start PyEos, PyEos will create a testing wallet for you, which placed in data-dir/mywallet.wallet, and then console will print the wallet password as below:

```
wallet password: PW5JWE5g6RZ7Fyr2kmCphDqZo4uivdeGpUpndgFZ52rsduhtf9PRJ
```

Since it's for testing only, password will save to data-dir/data.pkl, So next time you start pyeos for testing, pyeos will unlock wallet for you.

Also PyEos will import three private key to the wallet, which is useful for testing.

```
'5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3',
'5JEcwbckBCdmji5j8ZoMHLEUS8TqQiqBG1DRx1X9DN124GUok9s',
'5JbDP55GXN7MLcNYKCnJtfKi9aD2HvHAdY7g8m67zFTAFkY1uBB'
```

Keep in mind that these private key should never be used in real account, otherwise you will lose all of your crypto property in the account.

Beside that, PyEos will create four important accounts for you:

```
eosio.bios, eosio.msig, eosio.system, eosio.token
```

and publish their smart contract on testnet. 

Although the above steps will never happen in the real world, but it's really provide a great convenience for testing smart contract. Thus save a lot of your precious time and make the development more efficient.

### Generating source code with sketch

Run the following command in PyEos console,

```
sketch.create('hello', 'helloworld', 'py')
```

That will create a helloworld directory under your current directory with hello as the testing account name. There are three file generated in the directory:

```
helloworld.py
helloworld.abi
t.py
```

Which helloworld.py is the Python smart contract source code, helloworld.abi is the ABI(Application Binary Interface) file for smart contract, t.py contains code for testing smart contract.

In addition, sketch can also create a wasm smart contract project for you, just type the following code in PyEos console, and the testing process has no difference with Python smart contract.

```
sketch.create('helloworld', 'helloworld', 'cpp')
```

### Testing


Now it's time to run your helloworld smart contract program. Type or copy the following command to the PyEos console:

```python
from helloworld import t
t.test()
```

You will see the following output on console in green words:

```
3289633ms thread-1   mpeoslib.cpp:63               print                ] hello,world
```

Congratulations, you have successfully run your first Python smart contract.

Now you can open helloworld.py for coding. Once it's done, just run t.test() again, 
there is no need to run other command to publish your testing smart contract, the smart contract will be automatically
republish to the testnet if it's been changed during the running of t.test(). You can also edit the testing code in t.py for testing your smart contract. Once it's done, just run t.test() again, there is no need to run reload(t), PyEos will do the magic for you. That also works at the situation of adding a new function in test. 

<a name="smartcontractdebuggingcplusplus"></a>

# Debugging With C++ Smart Contract

On Eos, C++ Smart Contract code is compile to WebAssembly bytecode, that makes debugging C++ suffer. Fortunately now It's able to compile C++ Smart Contract to a shared library, that makes debugging a C++ Smart Contract as easy as debugging a normal C++ project. 

There is a short video on youtube for quick start:[C++ Smart Contract Debugging](https://youtu.be/7XPgnbjsXkE)

To be brief, here are the steps on debugging a C++ Smart Contract:

### 1. Open pyeos project in Visual Studio Code

### 2. Edit CMAKE_BUILD_TYPE and BUILD_DIR in eosio_build.sh

```
    BUILD_DIR="${PWD}/build-debug"
    CMAKE_BUILD_TYPE=Debug
```

### 3. Build pyeos in VSC terminal

```
./eosio_build.sh
```

### 4.Configure debug in Visual Studio Code
```
      {
         "name": "(lldb) Attach pyeos",
         "type": "cppdbg",
         "request": "attach",
         "program": "${workspaceFolder}/build-debug/programs/pyeos/pyeos",
         "processId": "${command:pickProcess}",
         "MIMode": "lldb"
      }
```

### 5. Launch pyeos

```
./pyeos/pyeos --manual-gen-block --debug -i  --contracts-console
```

### 6. Attach to pyeos

### 7. Create C++ Smart Contract test code.

```python
sketch.create('hello', 'helloworld', 'cpp')
```

### 8. Set breakpoint in test code

### 9. test
```
from helloworld import t
t.debug()
```

<a name="smartcontractdebuggingpython"></a>

# Python Smart Contract Debugging

There is a short video on youtube about Python Smart Contract for quick start:[Python Smart Contract Debugging](https://youtu.be/eTwx-VTxhfo)

The following steps show how to debug smart contract under programs/pyeos/contracts/hello.

#### 1. Launch PyEos
```
./pyeos/pyeos --manual-gen-block --debug -i  --contracts-console
```

#### 2. Set debug contract
```
debug.set_debug_contract('hello', '../../programs/pyeos/contracts/hello/hello.py')
```

#### 3. Start ptvsd debugger
```python
import ptvsd
ptvsd.enable_attach("12345", address = ('127.0.0.1', 3000))
ptvsd.wait_for_attach()
```

#### 4. Attach to ptvsd debugger in Visual Studio Code
Here is the debug setting:

```javascript
   {
      "name": "python Attach (Remote Debug)",
      "type": "python",
      "request": "attach",
      "localRoot": "${workspaceFolder}",
      "remoteRoot": "${workspaceFolder}",
      "port": 3000,
      "secret": "12345",
      "host": "localhost"
   },
```


#### 5. Set breakpoint in hello.py

#### 6. Debugging
```
from hello import t
t.test()
```

Enjoy it!

<a name="pyeosapioverview"></a>

# PyEos api overview

#### eosapi.get_info
```python
info = eosapi.get_info()
info
```

```javascript
{
    "server_version": "00000000",
    "head_block_num": 127,
    "last_irreversible_block_num": 126,
    "head_block_id": "0000007fbd1ff82f29668dfa89d927a0510c657cce292c033496ccaacf04c12a",
    "head_block_time": "2018-05-06T07:57:44",
    "head_block_producer": "eosio"
}
```

```python
info.head_block_time
```
```
'2017-09-23T15:16:18'
```

```python
info.head_block_num
```

```
18624
```

#### wallet.create

PyEos will create a testing wallet for you the first time you start it, but you can also create other wallet wit wallet.create API

```python
psw = wallet.create('mywallet2')
psw
```

```
'PW5JCWXaGkA15s6th6AWCabHewuGASAtrUJjTWoL1Ybx6sG9QzrSb'
```
You can see you wallet now. The * behind mywallet means the wallet is unlocked.

```
wallet.list_wallets()
```

```
['mywallet *']
```
#### wallet.open wallet.unlock

```
wallet.open('mywallet2')
wallet.unlock('mywallet2','PW5JCWXaGkA15s6th6AWCabHewuGASAtrUJjTWoL1Ybx6sG9QzrSb')
```

#### wallet.import_key
Let's import the private key of inita. Please refer to [Setting up a wallet and importing account key](https://github.com/learnforpractice/pyeos#setting-up-a-wallet-and-importing-account-key) for more information.

```
wallet.import_key('mywallet2','5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3')
```

#### eosapi.create_key

Create owner key

```python
eosapi.create_key()
```

```javascript
{'public': 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst', 'private': '5JEcwbckBCdmji5j8ZoMHLEUS8TqQiqBG1DRx1X9DN124GUok9s'}
```

Create active key

```python
eosapi.create_key()
```

```javascript
{'public': 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL', 'private': '5JbDP55GXN7MLcNYKCnJtfKi9aD2HvHAdY7g8m67zFTAFkY1uBB'}
```

Import owner key

```python
wallet.import_key('mywallet2','5JEcwbckBCdmji5j8ZoMHLEUS8TqQiqBG1DRx1X9DN124GUok9s')
```

Import active key

```python
wallet.import_key('mywallet2','5JbDP55GXN7MLcNYKCnJtfKi9aD2HvHAdY7g8m67zFTAFkY1uBB')
```

#### eosapi.create_account

```python
key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
eosapi.create_account('eosio', 'currency', key1, key2)

```

#### eosapi.get_account

```
eosapi.get_account('currency')
```

```javascript
{
    "account_name": "currency",
    "head_block_num": 43,
    "head_block_time": "2018-07-11T09:01:00.500",
    "privileged": false,
    "last_code_update": "1970-01-01T00:00:00.000",
    "created": "2018-07-11T09:01:00.500",
    "ram_quota": 65206,
    "net_weight": 10050,
    "cpu_weight": 10050,
    "net_limit": {
        "used": 0,
        "available": 62988768000,
        "max": 62988768000
    },
    "cpu_limit": {
        "used": 0,
        "available": 12013286400,
        "max": 12013286400
    },
    "ram_usage": 3446,
    "permissions": [
        {
            "perm_name": "active",
            "parent": "owner",
            "required_auth": {
                "threshold": 1,
                "keys": [
                    {
                        "key": "EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst",
                        "weight": 1
                    }
                ],
                "accounts": [],
                "waits": []
            }
        },
        {
            "perm_name": "owner",
            "parent": "",
            "required_auth": {
                "threshold": 1,
                "keys": [
                    {
                        "key": "EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL",
                        "weight": 1
                    }
                ],
                "accounts": [],
                "waits": []
            }
        }
    ],
    "total_resources": {
        "owner": "currency",
        "net_weight": "1.0050 EOS",
        "cpu_weight": "1.0050 EOS",
        "ram_bytes": 65206
    },
    "self_delegated_bandwidth": {
        "from": "currency",
        "to": "currency",
        "net_weight": "1.0050 EOS",
        "cpu_weight": "1.0050 EOS"
    },
    "refund_request": null,
    "voter_info": {
        "owner": "currency",
        "proxy": "",
        "producers": [],
        "staked": 20100,
        "last_vote_weight": 0.0,
        "proxied_vote_weight": 0.0,
        "is_proxy": 0
    }
}
```

#### eosapi.get_balance
```
eosapi.get_balance('eosio')
```
return:
```
9999999961645.494
```

#### eosapi.transfer
```
eosapi.transfer('eosio', 'currency', 100.6666)
```
return:
```
True
```

#### eosapi.push_action
```python
eosapi.get_balance('eosio')
eosapi.get_balance('currency')
args = {"from":'eosio', "to":'currency', "quantity":'100.6666 EOS', "memo":'hello'}
r = eosapi.push_action('eosio.token', 'transfer', args, {'eosio':'active'})
eosapi.get_balance('eosio')
eosapi.get_balance('currency')
```

```python
>>> eosapi.get_balance('eosio')
9999999961142.162
>>> eosapi.get_balance('currency')
513.9996
>>> args = {"from":'eosio', "to":'currency', "quantity":'100.6666 EOS', "memo":'hello'}
>>> r = eosapi.push_action('eosio.token', 'transfer', args, {'eosio':'active'})
>>> eosapi.get_balance('eosio')
9999999961041.496
>>> eosapi.get_balance('currency')
614.6662
```

the above code is what eosapi.transfer does.

#### eosapi.push_actions
```python
r = get_balance('currency')
print(r)
args = {"from":'eosio', "to":'currency', "quantity":'100.6666 EOS', "memo":'hello'}
args = eosapi.pack_args('eosio.token', 'transfer', args)
act = ['eosio.token', 'transfer', {'eosio':'active'}, args]
r = eosapi.push_actions([act, act])
r = eosapi.get_balance('currency')
print(r)
```

```python
>>> r = get_balance('currency')
>>> print(r)
211.9998
>>> args = {"from":'eosio', "to":'currency', "quantity":'100.6666 EOS', "memo":'hello'}
>>> args = eosapi.pack_args('eosio.token', 'transfer', args)
>>> act = ['eosio.token', 'transfer', {'eosio':'active'}, args]
>>> r = eosapi.push_actions([act, act])
>>> r = eosapi.get_balance('currency')
>>> print(r)
413.333
```


#### eosapi.get_transaction

```python
r = eosapi.get_transaction('f6c43148dfac54105031fbaf966958d36309dd94e665c506eb2769e43febedba')
r
r.transaction.signatures
r.transaction.packed_trx
```

#### eosapi.set_contract
Publish python smart contract to the blockchain

```python
r = eosapi.set_contract('hello','../../programs/pyeos/contracts/hello/hello.py','../../contracts/hello/hello.abi',1)
r
```
