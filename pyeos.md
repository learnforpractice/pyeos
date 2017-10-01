Hack on Eos. Have fun!

# What does this project for?

Eos have the potential of been a revolutionary technology. I'm curious about the tech behind Eos. So I think I need to do something as early as possible.
Python is one of the most powerfull language on the earth. It's easy to use and easy to understand,also it's easy to interface with other program language such as c++,c etc. You can do a lot of things with python without paying too much time and too much energy. But for the reason of permarmance,Eos is writing in C++. So I think maybe I can rewrite Eos in python? At least I can improve the usability of Eos with python. So here comes pyeos.

# What pyeos can do?

1. call eos api with python
2. write simple contract in python. You can find some demos in directory [contract](https://github.com/learnforpractice/eos/tree/master/programs/pyeos/contract). Anyone who want faster code can also code it in python first and then porting it to C++.


# How to build

You have to export two environment before build,one telling the compiler where to find the python header file,the other telling the linker where to find the python library.

```bash
export PYTHON_INC_DIR="~/anaconda/include/python3.6m"
export PYTHON_LIB="~/anaconda/lib/libpython3.6m.dylib"

```

Also you shall have cython installed. If you don't,install it by the following command:

```
python -m pip install cython
```

or

```
pip install cython
```

Now following the instruction on [Building EOS and running a node](https://github.com/learnforpractice/eos#runanode)

Currently only test with python 3.6 on macOS 10.12.6


# How to run

After successful build of project,run the following commands to start pyeos.

```
$ export PYTHONHOME=~/anaconda/
$ export PYTHONPATH='~/dev/eos/programs/pyeos/'
$ cd ~/dev/eos/build/programs/
$ ./pyeos/pyeos -i
```
The first three commands only need to run once. If everything is fine,a python interactive console shall appeal.That's it. Next,let us see what pyeos can do.


# let's do some practices
#### eosapi.get_info
```python
>>> info = eosapi.get_info()
>>> info
```

```
{'head_block_num': 18624, 'last_irreversible_block_num': 18610, 'head_block_id': '000048c0b53d0e7f4ec3f41e28fe78a10f2a1de36cc6fdae79493314c7ea982c', 'head_block_time': '2017-09-23T15:16:18', 'head_block_producer': 'inito', 'recent_slots': '1111111111100000000000000000000000000000000000000000000011111111', 'participation_rate': 0}
```

```python
>>> info.head_block_time
```
```
'2017-09-23T15:16:18'
```

```python
>>> info.head_block_num
```
```
18624
```
#### wallet.create

Let's create a wallet first,PW5JCWXaGkA15s6th6AWCabHewuGASAtrUJjTWoL1Ybx6sG9QzrSb is wallet password,save it in some safe place,if you forgot your wallet password or the password was compromised,you will lose everything in the wallet.Next time you unlock your wallet,a corrent password must be provided.
```python
>>> psw = wallet.create('mywallet')
>>> psw
```
```
'PW5JCWXaGkA15s6th6AWCabHewuGASAtrUJjTWoL1Ybx6sG9QzrSb'
```
You can see you wallet now. The * behind mywallet means the wallet is unlocked.
```
>>> wallet.list_wallets()
['mywallet *']
```
#### wallet.open wallet.unlock

Next time you start pyeos,you will need to type the following command to load and unlock your wallet.You will need to change the password PW5JCWXaGkA15s6th6AWCabHewuGASAtrUJjTWoL1Ybx6sG9QzrSb below to the password you created before.

```
>>> wallet.open('mywallet')
True
>>> wallet.unlock('mywallet','PW5JCWXaGkA15s6th6AWCabHewuGASAtrUJjTWoL1Ybx6sG9QzrSb')
True
```
#### wallet.import_key
Let's import the private key of inita. Please refer to [Setting up a wallet and importing account key](https://github.com/learnforpractice/pyeos#setting-up-a-wallet-and-importing-account-key) for more information.
```
>>> wallet.import_key('mywallet','5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3')
True
```
#### eosapi.create_key

Create owner key
```python
>>> eosapi.create_key()
```
```
{'public': 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst', 'private': '5JEcwbckBCdmji5j8ZoMHLEUS8TqQiqBG1DRx1X9DN124GUok9s'}
```
Create active key
```python
>>> eosapi.create_key()
```
```
{'public': 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL', 'private': '5JbDP55GXN7MLcNYKCnJtfKi9aD2HvHAdY7g8m67zFTAFkY1uBB'}
```

Import owner key
```python
>>> wallet.import_key('mywallet','5JEcwbckBCdmji5j8ZoMHLEUS8TqQiqBG1DRx1X9DN124GUok9s')
```
```
True
```
Import active key
```python
>>> wallet.import_key('mywallet','5JbDP55GXN7MLcNYKCnJtfKi9aD2HvHAdY7g8m67zFTAFkY1uBB')
```
```
True
```
#### eosapi.create_account
It's time to create a account,key1 and key2 are the public key you created before.
```
>>> key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
>>> key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
>>> r = eosapi.create_account('inita', 'currency',key1,key2)
>>> r
```
```
{'transaction_id': '9c43ad2e28ae7f2685c271606834d6b3051e800b45552bc8d84ea9928ca75841', 'processed': {'refBlockNum': 534, 'refBlockPrefix': 3759658563, 'expiration': '2017-09-24T02:49:19', 'scope': ['eos', 'inita'], 'signatures': ['1f073449e507043731a42a31a1d02a6d83a74e0b91beb8e0407cfeb3e1e924daf447ceb4dd9b092f479d09416ef78cb1baa8ca0e5e8243c1ce58fca7b685ba9d94'], 'messages': [{'code': 'eos', 'type': 'newaccount', 'authorization': [{'account': 'inita', 'permission': 'active'}], 'data': {'creator': 'inita', 'name': 'currency', 'owner': {'threshold': 1, 'keys': [{'key': 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst', 'weight': 1}], 'accounts': []}, 'active': {'threshold': 1, 'keys': [{'key': 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL', 'weight': 1}], 'accounts': []}, 'recovery': {'threshold': 1, 'keys': [], 'accounts': [{'permission': {'account': 'inita', 'permission': 'active'}, 'weight': 1}]}, 'deposit': '0.0001 EOS'}, 'hex_data': '000000008040934b00000079b822651d010000000102934a4748562795f31685de7b0112c3f4428255d42aa8ea420701c29542ede465010000010000000102376f7109de7a6cf12a8d6c713fa7f4d4df8a9e08ecee47e9001583c7b8fcc975010000010000000001000000008040934b00000000149be8080100010000000000000004454f5300000000'}], 'output': [{'notify': [], 'deferred_transactions': []}]}}
```


#### eosapi.get_transaction
```python
>>> r = eosapi.get_transaction('9c43ad2e28ae7f2685c271606834d6b3051e800b45552bc8d84ea9928ca75841')
>>> r
```
```
{'transaction_id': '9c43ad2e28ae7f2685c271606834d6b3051e800b45552bc8d84ea9928ca75841', 'transaction': {'refBlockNum': 534, 'refBlockPrefix': 3759658563, 'expiration': '2017-09-24T02:49:19', 'scope': ['eos', 'inita'], 'signatures': ['1f073449e507043731a42a31a1d02a6d83a74e0b91beb8e0407cfeb3e1e924daf447ceb4dd9b092f479d09416ef78cb1baa8ca0e5e8243c1ce58fca7b685ba9d94'], 'messages': [{'code': 'eos', 'type': 'newaccount', 'authorization': [{'account': 'inita', 'permission': 'active'}], 'data': {'creator': 'inita', 'name': 'currency', 'owner': {'threshold': 1, 'keys': [{'key': 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst', 'weight': 1}], 'accounts': []}, 'active': {'threshold': 1, 'keys': [{'key': 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL', 'weight': 1}], 'accounts': []}, 'recovery': {'threshold': 1, 'keys': [], 'accounts': [{'permission': {'account': 'inita', 'permission': 'active'}, 'weight': 1}]}, 'deposit': '0.0001 EOS'}, 'hex_data': '000000008040934b00000079b822651d010000000102934a4748562795f31685de7b0112c3f4428255d42aa8ea420701c29542ede465010000010000000102376f7109de7a6cf12a8d6c713fa7f4d4df8a9e08ecee47e9001583c7b8fcc975010000010000000001000000008040934b00000000149be8080100010000000000000004454f5300000000'}], 'output': [{'notify': [], 'deferred_transactions': []}]}}
```
```python
>>> r.transaction
```
```
{'refBlockNum': 534, 'refBlockPrefix': 3759658563, 'expiration': '2017-09-24T02:49:19', 'scope': ['eos', 'inita'], 'signatures': ['1f073449e507043731a42a31a1d02a6d83a74e0b91beb8e0407cfeb3e1e924daf447ceb4dd9b092f479d09416ef78cb1baa8ca0e5e8243c1ce58fca7b685ba9d94'], 'messages': [{'code': 'eos', 'type': 'newaccount', 'authorization': [{'account': 'inita', 'permission': 'active'}], 'data': {'creator': 'inita', 'name': 'currency', 'owner': {'threshold': 1, 'keys': [{'key': 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst', 'weight': 1}], 'accounts': []}, 'active': {'threshold': 1, 'keys': [{'key': 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL', 'weight': 1}], 'accounts': []}, 'recovery': {'threshold': 1, 'keys': [], 'accounts': [{'permission': {'account': 'inita', 'permission': 'active'}, 'weight': 1}]}, 'deposit': '0.0001 EOS'}, 'hex_data': '000000008040934b00000079b822651d010000000102934a4748562795f31685de7b0112c3f4428255d42aa8ea420701c29542ede465010000010000000102376f7109de7a6cf12a8d6c713fa7f4d4df8a9e08ecee47e9001583c7b8fcc975010000010000000001000000008040934b00000000149be8080100010000000000000004454f5300000000'}], 'output': [{'notify': [], 'deferred_transactions': []}]}
```
```python
>>> r.transaction.refBlockNum
```
```
534
```
```python
>>> r.transaction.messages
```
```
[{'code': 'eos', 'type': 'newaccount', 'authorization': [{'account': 'inita', 'permission': 'active'}], 'data': {'creator': 'inita', 'name': 'currency', 'owner': {'threshold': 1, 'keys': [{'key': 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst', 'weight': 1}], 'accounts': []}, 'active': {'threshold': 1, 'keys': [{'key': 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL', 'weight': 1}], 'accounts': []}, 'recovery': {'threshold': 1, 'keys': [], 'accounts': [{'permission': {'account': 'inita', 'permission': 'active'}, 'weight': 1}]}, 'deposit': '0.0001 EOS'}, 'hex_data': '000000008040934b00000079b822651d010000000102934a4748562795f31685de7b0112c3f4428255d42aa8ea420701c29542ede465010000010000000102376f7109de7a6cf12a8d6c713fa7f4d4df8a9e08ecee47e9001583c7b8fcc975010000010000000001000000008040934b00000000149be8080100010000000000000004454f5300000000'}]
```
#### eosapi.get_account
```python
>>> info = eosapi.get_account('eos')
>>> info
```
```
{'name': 'eos', 'balance': 690000000000, 'stakedBalance': 0, 'unstakingBalance': 0, 'lastUnstakingTime': '2106-02-07T06:28:15', 'abi': '{"types":[{"newTypeName":"AccountName","type":"Name"},{"newTypeName":"ShareType","type":"Int64"}],"structs":[{"name":"transfer","base":"","fields":{"from":"AccountName","to":"AccountName","amount":"UInt64","memo":"String"}},{"name":"lock","base":"","fields":{"from":"AccountName","to":"AccountName","amount":"ShareType"}},{"name":"unlock","base":"","fields":{"account":"AccountName","amount":"ShareType"}},{"name":"claim","base":"","fields":{"account":"AccountName","amount":"ShareType"}},{"name":"okproducer","base":"","fields":{"voter":"AccountName","producer":"AccountName","approve":"Int8"}},{"name":"setproducer","base":"","fields":{"name":"AccountName","key":"PublicKey","configuration":"BlockchainConfiguration"}},{"name":"setproxy","base":"","fields":{"stakeholder":"AccountName","proxy":"AccountName"}},{"name":"updateauth","base":"","fields":{"account":"AccountName","permission":"PermissionName","parent":"PermissionName","authority":"Authority"}},{"name":"linkauth","base":"","fields":{"account":"AccountName","code":"AccountName","type":"FuncName","requirement":"PermissionName"}},{"name":"unlinkauth","base":"","fields":{"account":"AccountName","code":"AccountName","type":"FuncName"}},{"name":"deleteauth","base":"","fields":{"account":"AccountName","permission":"PermissionName"}},{"name":"newaccount","base":"","fields":{"creator":"AccountName","name":"AccountName","owner":"Authority","active":"Authority","recovery":"Authority","deposit":"Asset"}}],"actions":[{"action":"transfer","type":"transfer"},{"action":"lock","type":"lock"},{"action":"unlock","type":"unlock"},{"action":"claim","type":"claim"},{"action":"okproducer","type":"okproducer"},{"action":"setproducer","type":"setproducer"},{"action":"setproxy","type":"setproxy"},{"action":"linkauth","type":"linkauth"},{"action":"unlinkauth","type":"unlinkauth"},{"action":"updateauth","type":"updateauth"},{"action":"deleteauth","type":"deleteauth"},{"action":"newaccount","type":"newaccount"}],"tables":[]}'}
```
```python
>>> info.balance
```
```
690000000000
```
#### eosapi.set_contract
Here comes the most exciting moment. Let's push a python smart contract to the blockchain.You can find the source file in [contract](https://github.com/learnforpractice/pyeos/tree/master/programs/pyeos/contract) directory
```python
>>> r = eosapi.set_contract('currency','../../programs/pyeos/contract/currency.py','../../contracts/currency/currency.abi',1,True)
>>> r
```

```
{'transaction_id': '108f182e02ca0e9dd9ca40f883dfd7c922298b5a436eef3c229ee231be9217be', 'processed': {'refBlockNum': 1815, 'refBlockPrefix': 3841583051, 'expiration': '2017-09-24T03:53:22', 'scope': ['currency', 'eos'], 'signatures': ['200ab5c4776adf7b66f4cdce8af88a65d09fe7c85df1e003c6fd8aeeb4a145aecd118d7162526ba6fa4ed36c85fde907b8218335dfcc432b15d6574485865a5328'], 'messages': [{'code': 'eos', 'type': 'setcode', 'authorization': [{'account': 'currency', 'permission': 'active'}], 'data': '00000079b822651d0100e5096966205f5f6e616d655f5f203d3d20275f5f6d61696e5f5f273a0a09696d706f727420656f736c69625f64756d6d7920617320656f736c69620a656c73653a0a09696d706f727420656f736c69620a696d706f7274207374727563740a0a636f6465203d20656f736c69622e4e28622763757272656e637927290a7461626c65203d20656f736c69622e4e2862276163636f756e7427290a0a636c617373204163636f756e74286f626a656374293a0a096b6579203d20656f736c69622e4e2862276163636f756e7427290a09646566205f5f696e69745f5f2873656c662c73636f70652c62616c616e63653d30293a0a090973656c662e73636f7065203d2073636f70650a090969662062616c616e6365203d3d20303a0a09090973656c662e6c6f616428290a0909656c73653a0a09090973656c662e62616c616e6365203d2062616c616e63650a09646566206973456d7074792873656c66293a0a090972657475726e2062616c616e6365203d3d20300a096465662073746f72652873656c66293a0a0909656f736c69622e73746f72655f7536342873656c662e73636f70652c636f64652c7461626c652c4163636f756e742e6b65792c73656c662e62616c616e6365290a09646566206c6f61642873656c66293a0a090973656c662e62616c616e6365203d20656f736c69622e6c6f61645f7536342873656c662e73636f70652c636f64652c7461626c652c4163636f756e742e6b6579290a0a64656620696e697428293a0a23097072696e74282768656c6c6f2066726f6d20696e697427290a0961203d204163636f756e7428636f64652c313030303030290a09612e73746f726528290a0a646566206170706c79286e616d652c74797065293a0a23097072696e74282768656c6c6f2066726f6d20707974686f6e206170706c79272c6e616d652c74797065290a23097072696e7428656f736c69622e6e3273286e616d65292c656f736c69622e6e3273287479706529290a0969662074797065203d3d20656f736c69622e4e2862277472616e7366657227293a0a09096d7367203d20656f736c69622e726561644d65737361676528290a0909726573756c74203d207374727563742e756e7061636b2827515151272c6d7367290a2309097072696e7428726573756c74290a090966726f6d5f203d20726573756c745b305d0a0909746f5f203d20726573756c745b315d0a0909616d6f756e74203d20726573756c745b325d0a09090a0909656f736c69622e7265717569726541757468282066726f6d5f20293b0a0909656f736c69622e726571756972654e6f74696365282066726f6d5f20293b0a0909656f736c69622e726571756972654e6f746963652820746f5f20290a0a090966726f6d5f203d204163636f756e742866726f6d5f290a0909746f5f203d204163636f756e7428746f5f290a090969662066726f6d5f2e62616c616e6365203e3d20616d6f756e743a0a09090966726f6d5f2e62616c616e6365202d3d20616d6f756e740a090909746f5f2e62616c616e6365202b3d20616d6f756e740a09090966726f6d5f2e73746f726528290a090909746f5f2e73746f726528290a0a6966205f5f6e616d655f5f203d3d20275f5f6d61696e5f5f273a0a09696e697428290a096170706c7928656f736c69622e4e286227707974686f6e27292c656f736c69622e4e2862277472616e736665722729290a0a0a0a010b4163636f756e744e616d65044e616d6502087472616e7366657200030466726f6d0b4163636f756e744e616d6502746f0b4163636f756e744e616d6506616d6f756e740655496e743634076163636f756e740002076163636f756e74044e616d650762616c616e63650655496e74363401000000b298e982a4087472616e736665720100000080bafac6080369363401076163636f756e7400076163636f756e74'}], 'output': [{'notify': [], 'deferred_transactions': []}]}}
```

```python
>>> r = eosapi.get_account('currency')
>>> r
```
```
{'name': 'currency', 'balance': 0, 'stakedBalance': 1, 'unstakingBalance': 0, 'lastUnstakingTime': '2106-02-07T06:28:15', 'abi': '{"types":[{"newTypeName":"AccountName","type":"Name"}],"structs":[{"name":"transfer","base":"","fields":{"from":"AccountName","to":"AccountName","amount":"UInt64"}},{"name":"account","base":"","fields":{"account":"Name","balance":"UInt64"}}],"actions":[{"action":"transfer","type":"transfer"}],"tables":[{"table":"account","indextype":"i64","keynames":["account"],"keytype":[],"type":"account"}]}'}
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
>>> eosapi.get_table('currency','currency','account')
```
```
{'rows': [{'account': 'account', 'balance': 100000}], 'more': False}
```
```python
>>> r = eosapi.get_table('currency','currency','account')
>>> r
```

```
{'rows': [{'account': 'account', 'balance': 100000}], 'more': False}
```

```
>>> r.rows[0].balance
```
```
100000
```
#### eosapi.push_message
Let's send some money to inita
```python
>>> r = eosapi.push_message('currency','transfer','{"from":"currency","to":"inita","amount":50}',['currency','inita'],{'currency':'active'})
```
```python
>>> eosapi.get_table('currency','currency','account')
```
```
{'rows': [{'account': 'account', 'balance': 99950}], 'more': False}
```
Let's see how much balances inita has now.
```python
>>> eosapi.get_table('inita','currency','account')
```
```
{'rows': [{'account': 'account', 'balance': 50}], 'more': False}
```

to be continued...

