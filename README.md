A Self Evolving Universal Smart Contract Platform Base on The Development of EOSIO 
# Table of contents
1. [Building PyEos](#buildingpyeos)
2. [Creating Your First Python Smart Contract](#creatsmartcontract)
3. [Debugging With Python Smart Contract](#smartcontractdebuggingpython)
4. [Debugging With C++ Smart Contract](#smartcontractdebuggingcplusplus)
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
./pyeos/pyeos --manual-gen-block --debug -i
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

There are a lot of examples in programs/pyeos/contracts. Some of them are still in develop, if the example throws exception, then it's probably not done yet. Pick up an example you interest in and play with it as you want. 

<a name="smartcontractdebuggingpython"></a>

# Smart Contract Debugging

Python smart contract support source level debugging. Developers can debug their smart contract in Eclipse IDE with pydevd Eclipse pluging, other IDE such as Visual Studio Code may also be supported, Please search for online resources to find out how to debug Python source remotely. for pydevd, there is a reference from here [manual_adv_remote_debugger](http://www.pydev.org/manual_adv_remote_debugger.html). After the environment has been successfully setting up, run the following command in PyEos console to enable debugging.

```
debug.enable()
```

Set breakpoint at your python smart contract source code in Eclipse. If your Python smart contract source code does not placed in pyeos/contracts directory, them you need to run the following code to specify your source code directory and your smart contract source code file must under directory with the same name.

```
 sys.path.append(<folder where source code directory in>)
```

Deploy your smart contract to the testnet, Use hello contract in pyeos/contracts as example, set breakpoint at hello.py, and run the following code to call hello.py

```
from hello import t
t.test()
```

Smart contract execution will be stopped when the code at the line of breakpoint is being executed.


To disable debugging, run the following code.

```
debug.disable()
```

![Smart Contract Debugging](https://github.com/learnforpractice/pyeos/blob/master/programs/pyeos/debugging/debugging.png)

<a name="smartcontractdebuggingcplusplus"></a>

# Debugging With C++ Smart Contract

On Eos, C++ Smart Contract code is compile to WebAssembly bytecode, that makes debugging C++ suffer. Fortunately now It's able to compile C++ Smart Contract to a shared library, that makes debugging a C++ Smart Contract as easy as debugging a normal C++ project. 

There is a short video on youtube for quick start:[Smart Contract Debugging](https://youtu.be/7XPgnbjsXkE)

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
./pyeos/pyeos --manual-gen-block --debug -i
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

Enjoy it


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

```
{'public': 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst', 'private': '5JEcwbckBCdmji5j8ZoMHLEUS8TqQiqBG1DRx1X9DN124GUok9s'}
```

Create active key

```python
eosapi.create_key()
```

```
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
It's time to create an account, key1 and key2 are the public key you created before.

```
key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
eosapi.create_account('eosio', 'currency',key1,key2)

```

```javascript
{
    "transaction_id": "f6c43148dfac54105031fbaf966958d36309dd94e665c506eb2769e43febedba",
    "processed": {
        "status": "executed",
        "kcpu_usage": 100,
        "net_usage_words": 44,
        "id": "f6c43148dfac54105031fbaf966958d36309dd94e665c506eb2769e43febedba",
        "action_traces": [
            {
                "receiver": "eosio",
                "context_free": false,
                "cpu_usage": 0,
                "act": {
                    "account": "eosio",
                    "name": "newaccount",
                    "authorization": [
                        {
                            "actor": "eosio",
                            "permission": "active"
                        }
                    ],
                    "data": {
                        "creator": "eosio",
                        "name": "currency",
                        "owner": {
                            "threshold": 1,
                            "keys": [
                                {
                                    "key": "EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst",
                                    "weight": 1
                                }
                            ],
                            "accounts": []
                        },
                        "active": {
                            "threshold": 1,
                            "keys": [
                                {
                                    "key": "EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL",
                                    "weight": 1
                                }
                            ],
                            "accounts": []
                        },
                        "recovery": {
                            "threshold": 1,
                            "keys": [],
                            "accounts": [
                                {
                                    "permission": {
                                        "actor": "eosio",
                                        "permission": "active"
                                    },
                                    "weight": 1
                                }
                            ]
                        }
                    },
                    "hex_data": "0000000000ea30550000001e4d75af4601000000010002934a4748562795f31685de7b0112c3f4428255d42aa8ea420701c29542ede46501000001000000010002376f7109de7a6cf12a8d6c713fa7f4d4df8a9e08ecee47e9001583c7b8fcc9750100000100000000010000000000ea305500000000a8ed32320100"
                },
                "console": "",
                "data_access": [
                    {
                        "type": "write",
                        "code": "eosio",
                        "scope": "eosio.auth",
                        "sequence": 39
                    }
                ],
                "_profiling_us": 38
            }
        ],
        "deferred_transaction_requests": [],
        "read_locks": [],
        "write_locks": [
            {
                "account": "eosio",
                "scope": "eosio.auth"
            }
        ],
        "cpu_usage": 102400,
        "net_usage": 352,
        "packed_trx_digest": "373a3125c5728b7feb7a4b5d04330e9de64e5f393123e660ff210ca558864c99",
        "region_id": 0,
        "cycle_index": 1,
        "shard_index": 0,
        "_profiling_us": 66,
        "_setup_profiling_us": 127
    },
    "cost_time": 608
}
```
#### eosapi.produce_block

this command is used for generate a block when you are testing your smart contract.

```
eosapi.produce_block()
```

#### eosapi.get_transaction

we can test get_transaction now, replace 'f6c43148dfac54105031fbaf966958d36309dd94e665c506eb2769e43febedba' with the transcation id in the create_account output, you will notice the output is the same as the output of create_account command above.

```python
r = eosapi.get_transaction('f6c43148dfac54105031fbaf966958d36309dd94e665c506eb2769e43febedba')
r
r.transaction.signatures
r.transaction.packed_trx
```

#### eosapi.get_account

```python
info = eosapi.get_account('eosio')
info
```
```
{
    "account_name": "eosio",
    "permissions": [
        {
            "perm_name": "active",
            "parent": "owner",
            "required_auth": {
                "threshold": 1,
                "keys": [
                    {
                        "key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
                        "weight": 1
                    }
                ],
                "accounts": []
            }
        },
        {
            "perm_name": "owner",
            "parent": "",
            "required_auth": {
                "threshold": 1,
                "keys": [
                    {
                        "key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
                        "weight": 1
                    }
                ],
                "accounts": []
            }
        }
    ]
}
```

#### eosapi.set_contract
Publish python smart contract to the blockchain

```python
r = eosapi.set_contract('hello','../../programs/pyeos/contracts/hello/hello.py','../../contracts/hello/hello.abi',1)
r
```

#### eosapi.get_table

...

#### eosapi.push_action

...


