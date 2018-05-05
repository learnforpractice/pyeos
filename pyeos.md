An Experimental Project for Writing Smart Contract in Python, Base on The Development of EOSIO 
# Table of contents
1. [Building Pyeos](#buildingpyeos)
2. [Creating Your First Python Smart Contract](#creatsmartcontract)
3. [Pyeos api overview](#pyeosapioverview)

<a name="buildingpyeos"></a>

# Building Pyeos

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

### Generating source code with sketch.py

Open a terminal, cd to the source directory, run the following command to create you first project.

```
export PYEOS=$(pwd)
cd $PYEOS/programs/pyeos/contracts
python ../tools/sketch.py --account=hello --dir=helloworld
```

That will create a helloworld directory in programs/pyeos/contracts, with three files in the directory.

```
hello.py
hello.abi
test.py
```

Which hello.py is the Python smart contract source code, hello.abi is the ABI(Application Binary Interface) file for smart contract, test.py is for testing the smart contract, let's talk about it in detail later.

sketch.py can also create a cpp project for you, just run the following command

```
python ../tools/sketch.py --account=hello --dir=helloworld --lang=cpp
```

### Testing

Now, it's time to start Pyeos. Just run the following commands.

```
cd $PYEOS/build/programs
./pyeos/pyeos --manual-gen-block --debug -i
```

If it's the first time you start Pyeos, Pyeos will create a test wallet for you, which placed in data-dir/mywallet.wallet, and the console will print the wallet password as below:

```
wallet password: PW5JWE5g6RZ7Fyr2kmCphDqZo4uivdeGpUpndgFZ52rsduhtf9PRJ
```

Replace the password in programs/pyeos/initeos.py as shown below.

```
def init():
    psw = 'PW5K87AKbRvFFMJJm4dU7Zco4fi6pQtygEU4iyajwyTvmELUDnFBK'
```

So the next time you start Pyeos, wallet will be opened for you, and Pyeos is ready to run the test for you.

Beside that, Pyeos will create four important accounts for you:

```
eosio.bios, eosio.msig, eosio.system, eosio.token
```

and publish their smart contract on the testing blockchain. Although this can never happen in the real world, but it's really provide a great convenience for testing smart contract. Thus save a lot of your precious time and make the development more efficient.

Now it's time to run your helloworld smart contract program. Type or copy the following command to the python console,

```python
from helloworld import test
test.test()
```

You will see the following output in console in green words:

```
3289633ms thread-1   mpeoslib.cpp:63               print                ] hello,world
```

Congratulations, you are successfully running your first smart contract.

Now you can open hello.py for coding. Once it's done, just run test.test() again, 
their is no need to run other command to publish your testing smart contract.
You can also edit the testing code in test.py to test your smart contract. Once it's done, 
just run test.test() again, there is no need to run reload(test), Pyeos has do the magic for you.

There are a lot of examples in programs/pyeos/contracts, Some of them are still in development.
pick up an example you interest in and play with it as you want. 




<a name="pyeosapioverview"></a>

# Pyeos api overview

#### eosapi.get_info
```python
info = eosapi.get_info()
info
```

```javascript
{
    "head_block_num": 4,
    "last_irreversible_block_num": 0,
    "head_block_id": "00000004ef861f97d22241f96eafbf01700ad038fc03ba72f4390963b5cd048b",
    "head_block_time": "2017-10-11T13:16:24",
    "head_block_producer": "inite",
    "recent_slots": "1111111111111111111111111111111111111111111111111111111111111111",
    "participation_rate": 1.0
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

Let's create a wallet first, PW5JCWXaGkA15s6th6AWCabHewuGASAtrUJjTWoL1Ybx6sG9QzrSb is wallet password, save it in some safe place, if you forgot your wallet password or the password was compromised, you will lose everything in the wallet. Next time you unlock your wallet, a corrent password must be provided.

```python
psw = wallet.create('mywallet')
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

Next time you start Pyeos, you will need to type the following command to load and unlock your wallet. You will need to change the password below to the password you created before.

```
wallet.open('mywallet')
wallet.unlock('mywallet','PW5JCWXaGkA15s6th6AWCabHewuGASAtrUJjTWoL1Ybx6sG9QzrSb')
```

#### wallet.import_key
Let's import the private key of inita. Please refer to [Setting up a wallet and importing account key](https://github.com/learnforpractice/pyeos#setting-up-a-wallet-and-importing-account-key) for more information.

```
wallet.import_key('mywallet','5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3')
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
wallet.import_key('mywallet','5JEcwbckBCdmji5j8ZoMHLEUS8TqQiqBG1DRx1X9DN124GUok9s')
```

Import active key

```python
wallet.import_key('mywallet','5JbDP55GXN7MLcNYKCnJtfKi9aD2HvHAdY7g8m67zFTAFkY1uBB')
```

#### eosapi.create_account
It's time to create a account, key1 and key2 are the public key you created before.

```
key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
r = eosapi.create_account('inita', 'currency',key1,key2)
r
```

```javascript
{
    "transaction_id": "c1b808dd82ad6f299bf0ceaffce36e3134ed533e055fec83930b9b7108b2f4fc",
    "processed": {
        "refBlockNum": 238,
        "refBlockPrefix": 1273616980,
        "expiration": "2017-10-11T13:29:46",
        "scope": [
            "eos",
            "inita"
        ],
        "signatures": [
            "2053bb4fb2cc9aee1e6aa41a7bc4147c8ee8aaffa0352ac9d002f74aa660051dc404704d73f6443d1f650b935f6aedf22492e17ca2785c4209192cc2effafa5472"
        ],
        "messages": [
            {
                "code": "eos",
                "type": "newaccount",
                "authorization": [
                    {
                        "account": "inita",
                        "permission": "active"
                    }
                ],
                "data": {
                    "creator": "inita",
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
                                    "account": "inita",
                                    "permission": "active"
                                },
                                "weight": 1
                            }
                        ]
                    },
                    "deposit": "0.0001 EOS"
                },
                "hex_data": "000000008040934b00000079b822651d010000000102934a4748562795f31685de7b0112c3f4428255d42aa8ea420701c29542ede465010000010000000102376f7109de7a6cf12a8d6c713fa7f4d4df8a9e08ecee47e9001583c7b8fcc975010000010000000001000000008040934b00000000149be8080100010000000000000004454f5300000000"
            }
        ],
        "output": [
            {
                "notify": [],
                "deferred_transactions": []
            }
        ]
    }
}
```


#### eosapi.get_transaction

we can test get_transaction now, replace 'c1b808dd82ad6f299bf0ceaffce36e3134ed533e055fec83930b9b7108b2f4fc' with the transcation id in the create_account output, you will notice the output is the same as the output of create_account command above.

```python
r = eosapi.get_transaction('c1b808dd82ad6f299bf0ceaffce36e3134ed533e055fec83930b9b7108b2f4fc')
r
r.transaction
r.transaction.refBlockNum
r.transaction.messages
```

#### eosapi.get_account

```python
info = eosapi.get_account('eos')
info
```
```
{
    "name": "eos",
    "balance": 690000000000,
    "stakedBalance": 0,
    "unstakingBalance": 0,
    "lastUnstakingTime": "2106-02-07T06:28:15",
    "abi": "{\"types\":[{\"newTypeName\":\"AccountName\",\"type\":\"Name\"},{\"newTypeName\":\"ShareType\",\"type\":\"Int64\"}],\"structs\":[{\"name\":\"transfer\",\"base\":\"\",\"fields\":{\"from\":\"AccountName\",\"to\":\"AccountName\",\"amount\":\"UInt64\",\"memo\":\"String\"}},{\"name\":\"lock\",\"base\":\"\",\"fields\":{\"from\":\"AccountName\",\"to\":\"AccountName\",\"amount\":\"ShareType\"}},{\"name\":\"unlock\",\"base\":\"\",\"fields\":{\"account\":\"AccountName\",\"amount\":\"ShareType\"}},{\"name\":\"claim\",\"base\":\"\",\"fields\":{\"account\":\"AccountName\",\"amount\":\"ShareType\"}},{\"name\":\"okproducer\",\"base\":\"\",\"fields\":{\"voter\":\"AccountName\",\"producer\":\"AccountName\",\"approve\":\"Int8\"}},{\"name\":\"setproducer\",\"base\":\"\",\"fields\":{\"name\":\"AccountName\",\"key\":\"PublicKey\",\"configuration\":\"BlockchainConfiguration\"}},{\"name\":\"setproxy\",\"base\":\"\",\"fields\":{\"stakeholder\":\"AccountName\",\"proxy\":\"AccountName\"}},{\"name\":\"updateauth\",\"base\":\"\",\"fields\":{\"account\":\"AccountName\",\"permission\":\"PermissionName\",\"parent\":\"PermissionName\",\"authority\":\"Authority\"}},{\"name\":\"linkauth\",\"base\":\"\",\"fields\":{\"account\":\"AccountName\",\"code\":\"AccountName\",\"type\":\"FuncName\",\"requirement\":\"PermissionName\"}},{\"name\":\"unlinkauth\",\"base\":\"\",\"fields\":{\"account\":\"AccountName\",\"code\":\"AccountName\",\"type\":\"FuncName\"}},{\"name\":\"deleteauth\",\"base\":\"\",\"fields\":{\"account\":\"AccountName\",\"permission\":\"PermissionName\"}},{\"name\":\"newaccount\",\"base\":\"\",\"fields\":{\"creator\":\"AccountName\",\"name\":\"AccountName\",\"owner\":\"Authority\",\"active\":\"Authority\",\"recovery\":\"Authority\",\"deposit\":\"Asset\"}}],\"actions\":[{\"action\":\"transfer\",\"type\":\"transfer\"},{\"action\":\"lock\",\"type\":\"lock\"},{\"action\":\"unlock\",\"type\":\"unlock\"},{\"action\":\"claim\",\"type\":\"claim\"},{\"action\":\"okproducer\",\"type\":\"okproducer\"},{\"action\":\"setproducer\",\"type\":\"setproducer\"},{\"action\":\"setproxy\",\"type\":\"setproxy\"},{\"action\":\"linkauth\",\"type\":\"linkauth\"},{\"action\":\"unlinkauth\",\"type\":\"unlinkauth\"},{\"action\":\"updateauth\",\"type\":\"updateauth\"},{\"action\":\"deleteauth\",\"type\":\"deleteauth\"},{\"action\":\"newaccount\",\"type\":\"newaccount\"}],\"tables\":[]}"
}
```

```python
>>> info.balance
```
```
690000000000
```

#### eosapi.set_contract
Here comes the most exciting moment. Let's push a Python smart contract to the blockchain. You can find the source code in [contracts](https://github.com/learnforpractice/pyeos/tree/master/programs/pyeos/contracts) directory

```python
r = eosapi.set_contract('currency','../../programs/pyeos/contracts/currency/currency.py','../../contracts/currency/currency.abi',1,True)
r
```

```
{
    "transaction_id": "0dcc5b427dfe17a4c3a83382877ce15252cde30d7c6e063bc59ac5cbb1c14b6e",
    "processed": {
        "refBlockNum": 505,
        "refBlockPrefix": 1794215310,
        "expiration": "2017-10-11T13:43:07",
        "scope": [
            "currency",
            "eos"
        ],
        "signatures": [
            "1f1dbb315088218d45e1e8b5e63e5b00b45f8be0a36111f83d95088479464661321b102dfcec56e740c0ad5ab5ef6e3fb53c7affc509b65c62fef9c378791bcc5c"
        ],
        "messages": [
            {
                "code": "eos",
                "type": "setcode",
                "authorization": [
                    {
                        "account": "currency",
                        "permission": "active"
                    }
                ],
                "data": "00000079b822651d0100b70a6966205f5f6e616d655f5f203d3d20275f5f6d61696e5f5f273a0a09696d706f727420656f736c69625f64756d6d7920617320656f736c69620a656c73653a0a09696d706f727420656f736c69620a696d706f7274207374727563740a0a636f6465203d20656f736c69622e4e28622763757272656e637927290a7461626c65203d20656f736c69622e4e2862276163636f756e7427290a0a636c617373204163636f756e74286f626a656374293a0a096b6579203d20656f736c69622e4e2862276163636f756e7427290a09646566205f5f696e69745f5f2873656c662c73636f70652c62616c616e63653d30293a0a090973656c662e73636f7065203d2073636f70650a090969662062616c616e6365203d3d20303a0a09090973656c662e6c6f616428290a0909656c73653a0a09090973656c662e62616c616e6365203d2062616c616e63650a09646566206973456d7074792873656c66293a0a090972657475726e2073656c662e62616c616e6365203d3d20300a096465662073746f72652873656c66293a0a0909656f736c69622e73746f72655f7536342873656c662e73636f70652c636f64652c7461626c652c4163636f756e742e6b65792c73656c662e62616c616e6365290a09646566206c6f61642873656c66293a0a090973656c662e62616c616e6365203d20656f736c69622e6c6f61645f7536342873656c662e73636f70652c636f64652c7461626c652c4163636f756e742e6b6579290a0a64656620696e697428293a0a23097072696e74282768656c6c6f2066726f6d20696e697427290a0961203d204163636f756e7428636f6465290a092361766f6964206f76657277726974652062616c616e636520616c7265616479206578697374732e0a09696620612e62616c616e6365203d3d20303a0a0909612e62616c616e6365203d203130303030300a0909612e73746f726528290a0a646566206170706c79286e616d652c74797065293a0a23097072696e74282768656c6c6f2066726f6d20707974686f6e206170706c79272c6e616d652c74797065290a23097072696e7428656f736c69622e6e3273286e616d65292c656f736c69622e6e3273287479706529290a0969662074797065203d3d20656f736c69622e4e2862277472616e7366657227293a0a09096d7367203d20656f736c69622e726561644d65737361676528290a0909726573756c74203d207374727563742e756e7061636b2827515151272c6d7367290a2309097072696e7428726573756c74290a090966726f6d5f203d20726573756c745b305d0a0909746f5f203d20726573756c745b315d0a0909616d6f756e74203d20726573756c745b325d0a09090a0909656f736c69622e7265717569726541757468282066726f6d5f20293b0a0909656f736c69622e726571756972654e6f74696365282066726f6d5f20293b0a0909656f736c69622e726571756972654e6f746963652820746f5f20290a0a090966726f6d5f203d204163636f756e742866726f6d5f290a0909746f5f203d204163636f756e7428746f5f290a090969662066726f6d5f2e62616c616e6365203e3d20616d6f756e743a0a09090966726f6d5f2e62616c616e6365202d3d20616d6f756e740a090909746f5f2e62616c616e6365202b3d20616d6f756e740a09090966726f6d5f2e73746f726528290a090909746f5f2e73746f726528290a0a6966205f5f6e616d655f5f203d3d20275f5f6d61696e5f5f273a0a09696e697428290a096170706c7928656f736c69622e4e286227707974686f6e27292c656f736c69622e4e2862277472616e736665722729290a0a0a0a010b4163636f756e744e616d65044e616d6502087472616e7366657200030466726f6d0b4163636f756e744e616d6502746f0b4163636f756e744e616d6506616d6f756e740655496e743634076163636f756e740002076163636f756e74044e616d650762616c616e63650655496e74363401000000b298e982a4087472616e736665720100000080bafac6080369363401076163636f756e7400076163636f756e74"
            }
        ],
        "output": [
            {
                "notify": [],
                "deferred_transactions": []
            }
        ]
    }
}

```

```python
r = eosapi.get_account('currency')
r
```

```
{
    "name": "currency",
    "balance": 0,
    "stakedBalance": 1,
    "unstakingBalance": 0,
    "lastUnstakingTime": "2106-02-07T06:28:15",
    "abi": "{\"types\":[{\"newTypeName\":\"AccountName\",\"type\":\"Name\"}],\"structs\":[{\"name\":\"transfer\",\"base\":\"\",\"fields\":{\"from\":\"AccountName\",\"to\":\"AccountName\",\"amount\":\"UInt64\"}},{\"name\":\"account\",\"base\":\"\",\"fields\":{\"account\":\"Name\",\"balance\":\"UInt64\"}}],\"actions\":[{\"action\":\"transfer\",\"type\":\"transfer\"}],\"tables\":[{\"table\":\"account\",\"indextype\":\"i64\",\"keynames\":[\"account\"],\"keytype\":[],\"type\":\"account\"}]}"
}
```
```python
>>> r.balance
```
```
0
```

#### eosapi.get_table
You can use get_table to retrive how much balances a account has.

```python
eosapi.get_table('currency','currency','account')
```
```
{
    "rows": [
        {
            "account": "account",
            "balance": 100000
        }
    ],
    "more": false
}
```

```
>>> r.rows[0].balance
```
```
100000
```

#### eosapi.push_message
Let's send some "money" to inita

```python
r = eosapi.push_message('currency','transfer','{"from":"currency","to":"inita","amount":50}',['currency','inita'],{'currency':'active'})
```
```python
eosapi.get_table('currency','currency','account')
```
```
{
    "rows": [
        {
            "account": "account",
            "balance": 99950
        }
    ],
    "more": false
}
```
Let's see how much balances inita has now.

```python
eosapi.get_table('inita','currency','account')
```
```
{
    "rows": [
        {
            "account": "account",
            "balance": 50
        }
    ],
    "more": false
}
```
