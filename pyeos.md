Hack on Eos. Have fun!
# Table of contents
1. [What does this project for?](#projectfor)
2. [What Pyeos can do?](#whatpyeoscando)
3. [Building Pyeos](#buildingpyeos)
4. [Running Pyeos](#runningpyeos)
5. [Pyeos api overview](#pyeosapioverview)
6. [Playing with DEX](#playwithdex)
7. [Sending messages with smart contracts](#sendingmsginline)
8. [Sending transactions with smart contracts](#sendingtsinline)
9. [About memory and CPU used by smart contracts](#memoryandcpu)
10.[Q&A](#questionandanswer)


<a name="projectfor"></a>
# What does this project for?

Eos has the potential of be a revolutionary technology. I'm curious about the tech behind Eos. So I think I need to do something as early as possible.
Python is one of the most powerfull language on the earth. It's easy to use and easy to understand,also it's easy to interface with other program language such as c++,c etc. You can do a lot of things with Python without paying too much time and too much energy. But for the reason of performance, Eos is writing in C++. So I think maybe I can rewrite Eos in Python? At least I can improve the usability of Eos with Python. So here comes Pyeos.

<a name="whatpyeoscando"></a>
# What Pyeos can do?

1. Calling Eos api with Python
2. Writing contracts in Python. You can find some demos in directory [contracts](https://github.com/learnforpractice/pyeos/tree/master/programs/pyeos/contracts). Anyone who want faster code can also code it in Python first and then porting it to C++.

<a name="buildingpyeos"></a>
# Building Pyeos

You have to export two environment before building pyeos, one telling the compiler where to find the Python header file, the other telling the linker where to find the Python library.

```bash
export PYTHON_INC_DIR="~/anaconda/include/python3.6m"
export PYTHON_LIB="~/anaconda/lib/libpython3.6m.dylib"

```

and in order to fix "fatal error: 'libintl.h' file not found", do the following command,change the gettext version as you want.

```bash
ln -s /usr/local/Cellar/gettext/0.19.8.1/include/libintl.h /usr/local/include/.
ln -s /usr/local/Cellar/gettext/0.19.8.1/lib/libintl.* /usr/local/lib/
```

Also you shall have cython installed. If you don't,install it by the following command:

```
python -m pip install cython
```

or

```
pip install cython
```

Pyeos depend on a custom build of cpython, run the following commands to build it

```
git submodule add -b v3.6.3-for-pyeos https://github.com/learnforpractice/cpython libraries/python
cd libraries/python
./configure --prefix=$(pwd)/dist
./make -j4
make install
```


Now follow the instruction on [Building EOS and running a node](https://github.com/learnforpractice/pyeos#runanode)

Currently only test with Python 3.6 on macOS 10.12.6

<a name="aboutcodestyle"></a>
# About code style

Bad code style will affect the cooperation between developers. So there must be some rule for developers to follow. Here is my recommendation: [google cpp guide](https://google.github.io/styleguide/cppguide.html) Who use eclipse as their IDE can use the following tool to format their cpp code.
http://www.cppstyle.com/

Also in order to compatible with eos source code, Change the tab width to 3 spaces.

<a name="runningpyeos"></a>
# Running Pyeos

After build project successfully,run the following commands to start Pyeos.

```
cd ~/dev/eos/build/programs/
export PYTHONHOME=../../libraries/python
export PYTHONPATH=../../libraries/python/Lib
./pyeos/pyeos -i
```
The first three commands only need to run once. If everything is fine, a Python interactive console shall appeal. That's it. Next, let us see what Pyeos can do. For a better debug, replace 

```
./pyeos/pyeos -i
```
with
```
./pyeos/pyeos --debug -i
```


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

<a name="playwithdex"></a>
# Playing with DEX

Before we can play with decentralized exchange, a few work need to be done. Open test.py in [program/pyeos/contracts/exchange](https://github.com/learnforpractice/pyeos/tree/master/programs/pyeos/contracts/exchange), edit wallet password and keys in test.init as show below. We have created them in the [first part](https://github.com/learnforpractice/pyeos/blob/master/pyeos.md#walletcreate):

```python
def init():
    import time
    psw = 'PW5KTHfg4QA7wD1dZjbkpA97hEktDtQaip6hNNswWkmYo5pDK3CL1'
    wallet.open('mywallet')
    wallet.unlock('mywallet',psw)
    
    key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
    key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
```

Now run the following commands to play with dex:

```python
from contracts.exchange import test
test.init();
test.test_deposit()
test.test_withdraw()
test.test_bs()
```

You can always check the result with the following command

```
eosapi.get_table('exchange','exchange','account')
```

for more information on what test does, read code in [test.py](https://github.com/learnforpractice/pyeos/tree/master/programs/pyeos/contracts/exchange/test.py)

<a name="sendingmsginline"></a>
# Sending messages with smart contracts

You can find the demo in [contracts/test](https://github.com/learnforpractice/pyeos/blob/master/programs/pyeos/contracts/test)

Here is how to play with it.

First, create account test

```python
key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
r = eosapi.create_account('inita', 'test', key1, key2)
```

Then send some currency to test, because in the demo test will send 50 currency to inita
```python
r = eosapi.push_message('currency','transfer','{"from":"currency","to":"test","amount":1000}',['currency','test'],{'currency':'active'})
```

Now It's time to publish contract code to account test. Pay attention, the contract code is in code.py, not test.py.

```python
r = eosapi.set_contract('test','../../programs/pyeos/contracts/test/code.py','../../programs/pyeos/contracts/test/test.abi',1)
```

The last step, call function test_message in the contract

```
r = eosapi.push_message('test','testmsg','',['test','inita'],{'test':'active'},rawargs=True)
```

To see what happened, run the following commands:

```python
r = eosapi.get_table('test','currency','account')
print(r)
r = eosapi.get_table('inita','currency','account')
print(r)
```

Now let's go to the detail

in [code.py](https://github.com/learnforpractice/pyeos/blob/master/programs/pyeos/contracts/test/code.py):

```python
def test_message():
# '{"from":"currency","to":"test","amount":50}'
    data = struct.pack("QQQ", N(b'test'), N(b'inita'), 50)
    msghandle = eoslib.messageCreate(b'currency', b'transfer', data)
    eoslib.messageRequirePermission(msghandle, b'test', b'active')
    eoslib.messageSend(msghandle)
```

```python
def apply(code, action):
    if code == test:
        eoslib.requireAuth(test)
        if action == N(b'transfer'):
        ...
        elif action == N(b'testmsg'):
            test_message()
```

in [test.abi](https://github.com/learnforpractice/pyeos/blob/master/programs/pyeos/contracts/test/test.abi):

```javascript
  "actions": [
  ...
    ,{
      "action": "testmsg",
      "type": "Bytes"
    }
...
    ],
```

Ok, that's all, pretty easy, right?


<a name="sendingtsinline"></a>
# Sending transactions with smart contracts

Sending transactions are much the same as sending messages. Here is the code copy from code.py. Transactions will be included in the next generated block.

```python
def test_transaction():
    print("------------------test_transaction---------------")
    #transaction will apply in the next block
    tshandle = eoslib.transactionCreate()
    eoslib.transactionRequireScope(tshandle, b'test', 0)
    eoslib.transactionRequireScope(tshandle, b'inita', 0)

# '{"from":"test","to":"inita","amount":50}'
    data = struct.pack("QQQ", N(b'test'), N(b'inita'), 50)
    msghandle = eoslib.messageCreate(b'currency', b'transfer', data)
    eoslib.messageRequirePermission(msghandle, b'test', b'active')

    eoslib.transactionAddMessage(tshandle, msghandle)
    eoslib.transactionSend(tshandle)
```

and the call abi is as following in test.abi :

```javascript
  "actions": [
  ...
  ,{
      "action": "testts",
      "type": "Bytes"
    }
  ]
```

To test the code, run the following code:

```
r = eosapi.push_message('test','testts','',['test',],{'test':'active'},rawargs=True)
```

Also you can run the following command to see the results, just as what you did in the Sending messages section.

```python
r = eosapi.get_table('test','currency','account')
print(r)
r = eosapi.get_table('inita','currency','account')
print(r)
```




<a name="memoryandcpu"></a>
# About memory and CPU used by smart contracts

Now memory and CPU are limited in python smart contracts, the maximum memory used by single smart contract is limited to 1MB and maximum execution time is limited to 100ms. Please be aware of these when you write a smart contracts in python.

<a name="questionandanswer"></a>
# Q&A


to be continued...
