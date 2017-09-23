Hack on Eos. Have fun!

# What does this project for?

Eos have the potential of been a revolutionary technology. I'm curious about the tech behind Eos. So I think I need to do something.
Python is one of the most powerfull language on the earth. It's easy to use and easy to understand,also it's easy to interface with other program language such as c++,c etc. You can do a lot of things with python without paying too much time and too much energy. But for the reason of permarmance,Eos is writing in C++. So I think maybe I can rewrite Eos in python? At least I can improve the usability of Eos with python. So here comes pyeos.

# What pyeos can do?

1. now you can call eos api with python
2. now you can write simple contract in python. You can find some demos in directory [contract](https://github.com/learnforpractice/eos/tree/master/programs/pyeos/contract). Anyone who want faster code can also code it in python first and then porting it to C++.


# How to build

You have to export two environment before build,one telling the compiler where to find the python header file,the other telling the linker where to find the python library.

```bash
export PYTHON_INC_DIR="~/anaconda/include/python3.6m"
export PYTHON_LIB="~/anaconda/lib/libpython3.6m.dylib"

```

Also you shall have cython installed. If you don't,you can install it by the following command:

```
python -m pip install cython
```

or

```
pip -m install cython
```

Now following the instruction on [Building EOS and running a node](https://github.com/learnforpractice/eos#runanode)

currently only test with python 3.6 on macOS 0.12.6


# How to run

after your successful build of project,you can run the following commands to start pyeos.

```
$ export PYTHONHOME=~/anaconda/
$ export PYTHONPATH='~/dev/eos/programs/pyeos/'
$ cd ~/dev/eos/build/programs/
$ ./pyeos/pyeos --skip-transaction-signatures
```
the first three commands only need to run once. If everything is fine,a python interactive console shall appeal. you can type python code in the console.That's it. Next,let me show you what pyeos can do.


# let's do some practices

### >>> info = eosapi.get_info()
### >>> info
```
{'head_block_num': 18624, 'last_irreversible_block_num': 18610, 'head_block_id': '000048c0b53d0e7f4ec3f41e28fe78a10f2a1de36cc6fdae79493314c7ea982c', 'head_block_time': '2017-09-23T15:16:18', 'head_block_producer': 'inito', 'recent_slots': '1111111111100000000000000000000000000000000000000000000011111111', 'participation_rate': 0}
```

### >>> info.head_block_time
```
'2017-09-23T15:16:18'
```

### >>> info.head_block_num
```
18624
```

### >>> key = eosapi.create_key()
### >>> key
```
{'public': 'EOS7GzeWw4ggnrEsXj7Ait97vbWjk2hxyVHuMriN1t7CAqD7FaLpD', 'private': '5JSqv15YN7j5b8CtPNcaQZaBFQYXzq2rS9VdEbzxYq3eEwGJeV7'}
```
### >>> key.public
```
'EOS7GzeWw4ggnrEsXj7Ait97vbWjk2hxyVHuMriN1t7CAqD7FaLpD'
```
### >>> key.private
```
'5JSqv15YN7j5b8CtPNcaQZaBFQYXzq2rS9VdEbzxYq3eEwGJeV7'
```

### >>> key1 = 'EOS4toFS3YXEQCkuuw1aqDLrtHim86Gz9u3hBdcBw5KNPZcursVHq'
### >>> key2 = 'EOS6KdkmwhPyc2wxN9SAFwo2PU2h74nWs7urN1uRduAwkcns2uXsa'
### >>> r = eosapi.create_account('inita', 'currency',key1,key2,False)
### >>> r
```
{'transaction_id': '4712db719a8f106efab265d53b25c6ecbcde8f0bb85b145cdd9ddfc03d6bc9f5', 'processed': {'refBlockNum': 20108, 'refBlockPrefix': 1312863972, 'expiration': '2017-09-23T17:06:58', 'scope': ['eos', 'inita'], 'signatures': [], 'messages': [{'code': 'eos', 'type': 'newaccount', 'authorization': [{'account': 'inita', 'permission': 'active'}], 'data': {'creator': 'inita', 'name': 'currency', 'owner': {'threshold': 1, 'keys': [{'key': 'EOS4toFS3YXEQCkuuw1aqDLrtHim86Gz9u3hBdcBw5KNPZcursVHq', 'weight': 1}], 'accounts': []}, 'active': {'threshold': 1, 'keys': [{'key': 'EOS6KdkmwhPyc2wxN9SAFwo2PU2h74nWs7urN1uRduAwkcns2uXsa', 'weight': 1}], 'accounts': []}, 'recovery': {'threshold': 1, 'keys': [], 'accounts': [{'permission': {'account': 'inita', 'permission': 'active'}, 'weight': 1}]}, 'deposit': '0.0001 EOS'}, 'hex_data': '000000008040934b00000000000080a701000000010200b35ad060d629717bd3dbec82731094dae9cd7e9980c39625ad58fa7f9b654b010000010000000102bcca6347d828d4e1868b7dfa91692a16d5b20d0ee3d16a7ca2ddcc7f6dd03344010000010000000001000000008040934b00000000149be8080100010000000000000004454f5300000000'}], 'output': [{'notify': [], 
'deferred_transactions': []}]}}
```
### >>> r.processed.messages[0]
```
{'code': 'eos', 'type': 'newaccount', 'authorization': [{'account': 'inita', 'permission': 'active'}], 'data': {'creator': 'inita', 'name': 'currency', 'owner': {'threshold': 1, 'keys': [{'key': 'EOS4toFS3YXEQCkuuw1aqDLrtHim86Gz9u3hBdcBw5KNPZcursVHq', 'weight': 1}], 'accounts': []}, 'active': {'threshold': 1, 'keys': [{'key': 'EOS6KdkmwhPyc2wxN9SAFwo2PU2h74nWs7urN1uRduAwkcns2uXsa', 'weight': 1}], 'accounts': []}, 'recovery': {'threshold': 1, 'keys': [], 'accounts': [{'permission': {'account': 'inita', 'permission': 'active'}, 'weight': 1}]}, 'deposit': '0.0001 EOS'}, 'hex_data': '000000008040934b00000000000080a701000000010200b35ad060d629717bd3dbec82731094dae9cd7e9980c39625ad58fa7f9b654b010000010000000102bcca6347d828d4e1868b7dfa91692a16d5b20d0ee3d16a7ca2ddcc7f6dd03344010000010000000001000000008040934b00000000149be8080100010000000000000004454f5300000000'}
```
### >>> r.processed.messages[0].code
```
'eos'
```
### >>> r.processed.messages[0].type
```
'newaccount'
```
### >>> r.processed.messages[0].authorization
```
[{'account': 'inita', 'permission': 'active'}]
```
### >>> r.processed.messages[0].data.owner
```
{'threshold': 1, 'keys': [{'key': 'EOS4toFS3YXEQCkuuw1aqDLrtHim86Gz9u3hBdcBw5KNPZcursVHq', 'weight': 1}], 'accounts': []}
```
### >>> r.processed.messages[0].data.owner.keys
```
[{'key': 'EOS4toFS3YXEQCkuuw1aqDLrtHim86Gz9u3hBdcBw5KNPZcursVHq', 'weight': 1}]
```

### >>> r = eosapi.get_transaction('4712db719a8f106efab265d53b25c6ecbcde8f0bb85b145cdd9ddfc03d6bc9f5')
### >>> r
```
{'transaction_id': '4712db719a8f106efab265d53b25c6ecbcde8f0bb85b145cdd9ddfc03d6bc9f5', 'transaction': {'refBlockNum': 20108, 'refBlockPrefix': 1312863972, 'expiration': '2017-09-23T17:06:58', 'scope': ['eos', 'inita'], 'signatures': [], 'messages': [{'code': 'eos', 'type': 'newaccount', 'authorization': [{'account': 'inita', 'permission': 'active'}], 'data': {'creator': 'inita', 'name': 'currency', 'owner': {'threshold': 1, 'keys': [{'key': 'EOS4toFS3YXEQCkuuw1aqDLrtHim86Gz9u3hBdcBw5KNPZcursVHq', 'weight': 1}], 'accounts': []}, 'active': {'threshold': 1, 'keys': [{'key': 'EOS6KdkmwhPyc2wxN9SAFwo2PU2h74nWs7urN1uRduAwkcns2uXsa', 'weight': 1}], 'accounts': []}, 'recovery': {'threshold': 1, 'keys': [], 'accounts': [{'permission': {'account': 'inita', 'permission': 'active'}, 'weight': 1}]}, 'deposit': '0.0001 EOS'}, 'hex_data': '000000008040934b00000000000080a701000000010200b35ad060d629717bd3dbec82731094dae9cd7e9980c39625ad58fa7f9b654b010000010000000102bcca6347d828d4e1868b7dfa91692a16d5b20d0ee3d16a7ca2ddcc7f6dd03344010000010000000001000000008040934b00000000149be8080100010000000000000004454f5300000000'}], 'output': [{'notify': [], 'deferred_transactions': []}]}}
```
### >>> r.transaction
```
{'refBlockNum': 20108, 'refBlockPrefix': 1312863972, 'expiration': '2017-09-23T17:06:58', 'scope': ['eos', 'inita'], 'signatures': [], 'messages': [{'code': 'eos', 'type': 'newaccount', 'authorization': [{'account': 'inita', 'permission': 'active'}], 'data': {'creator': 'inita', 'name': 'currency', 'owner': {'threshold': 1, 'keys': [{'key': 'EOS4toFS3YXEQCkuuw1aqDLrtHim86Gz9u3hBdcBw5KNPZcursVHq', 'weight': 1}], 'accounts': []}, 'active': {'threshold': 1, 'keys': [{'key': 'EOS6KdkmwhPyc2wxN9SAFwo2PU2h74nWs7urN1uRduAwkcns2uXsa', 'weight': 1}], 'accounts': []}, 'recovery': {'threshold': 1, 'keys': [], 'accounts': [{'permission': {'account': 'inita', 'permission': 'active'}, 'weight': 1}]}, 'deposit': '0.0001 EOS'}, 'hex_data': '000000008040934b00000000000080a701000000010200b35ad060d629717bd3dbec82731094dae9cd7e9980c39625ad58fa7f9b654b010000010000000102bcca6347d828d4e1868b7dfa91692a16d5b20d0ee3d16a7ca2ddcc7f6dd03344010000010000000001000000008040934b00000000149be8080100010000000000000004454f5300000000'}], 'output': [{'notify': [], 'deferred_transactions': []}]}
```
### >>> r.transaction.refBlockNum
```
20108
```
### >>> r.transaction.messages
```
[{'code': 'eos', 'type': 'newaccount', 'authorization': [{'account': 'inita', 'permission': 'active'}], 'data': {'creator': 'inita', 'name': 'currency', 'owner': {'threshold': 1, 'keys': [{'key': 'EOS4toFS3YXEQCkuuw1aqDLrtHim86Gz9u3hBdcBw5KNPZcursVHq', 'weight': 1}], 'accounts': []}, 'active': {'threshold': 1, 'keys': [{'key': 'EOS6KdkmwhPyc2wxN9SAFwo2PU2h74nWs7urN1uRduAwkcns2uXsa', 'weight': 1}], 'accounts': []}, 'recovery': {'threshold': 1, 'keys': [], 'accounts': [{'permission': {'account': 'inita', 'permission': 'active'}, 'weight': 1}]}, 'deposit': '0.0001 EOS'}, 'hex_data': '000000008040934b00000000000080a701000000010200b35ad060d629717bd3dbec82731094dae9cd7e9980c39625ad58fa7f9b654b010000010000000102bcca6347d828d4e1868b7dfa91692a16d5b20d0ee3d16a7ca2ddcc7f6dd03344010000010000000001000000008040934b00000000149be8080100010000000000000004454f5300000000'}]
```

### >>> info = eosapi.get_account('eos')
### >>> info
```
{'name': 'eos', 'balance': 690000000000, 'stakedBalance': 0, 'unstakingBalance': 0, 'lastUnstakingTime': '2106-02-07T06:28:15', 'abi': '{"types":[{"newTypeName":"AccountName","type":"Name"},{"newTypeName":"ShareType","type":"Int64"}],"structs":[{"name":"transfer","base":"","fields":{"from":"AccountName","to":"AccountName","amount":"UInt64","memo":"String"}},{"name":"lock","base":"","fields":{"from":"AccountName","to":"AccountName","amount":"ShareType"}},{"name":"unlock","base":"","fields":{"account":"AccountName","amount":"ShareType"}},{"name":"claim","base":"","fields":{"account":"AccountName","amount":"ShareType"}},{"name":"okproducer","base":"","fields":{"voter":"AccountName","producer":"AccountName","approve":"Int8"}},{"name":"setproducer","base":"","fields":{"name":"AccountName","key":"PublicKey","configuration":"BlockchainConfiguration"}},{"name":"setproxy","base":"","fields":{"stakeholder":"AccountName","proxy":"AccountName"}},{"name":"updateauth","base":"","fields":{"account":"AccountName","permission":"PermissionName","parent":"PermissionName","authority":"Authority"}},{"name":"linkauth","base":"","fields":{"account":"AccountName","code":"AccountName","type":"FuncName","requirement":"PermissionName"}},{"name":"unlinkauth","base":"","fields":{"account":"AccountName","code":"AccountName","type":"FuncName"}},{"name":"deleteauth","base":"","fields":{"account":"AccountName","permission":"PermissionName"}},{"name":"newaccount","base":"","fields":{"creator":"AccountName","name":"AccountName","owner":"Authority","active":"Authority","recovery":"Authority","deposit":"Asset"}}],"actions":[{"action":"transfer","type":"transfer"},{"action":"lock","type":"lock"},{"action":"unlock","type":"unlock"},{"action":"claim","type":"claim"},{"action":"okproducer","type":"okproducer"},{"action":"setproducer","type":"setproducer"},{"action":"setproxy","type":"setproxy"},{"action":"linkauth","type":"linkauth"},{"action":"unlinkauth","type":"unlinkauth"},{"action":"updateauth","type":"updateauth"},{"action":"deleteauth","type":"deleteauth"},{"action":"newaccount","type":"newaccount"}],"tables":[]}'}
```
### >>> info.balance
```
690000000000
```


### >>> r = eosapi.set_contract('currency','../../programs/pyeos/contract/currency.py','../../contracts/currency/currency.abi',1,False)
### >>> r
```
{'transaction_id': 'd13ce64e2e3e1f29b6585a72ebe396e2696eaa8c9c874291d805c1b965b97b83', 'processed': {'refBlockNum': 20694, 'refBlockPrefix': 1435237610, 'expiration': '2017-09-23T17:47:04', 'scope': ['currency', 'eos'], 'signatures': [], 'messages': [{'code': 'eos', 'type': 'setcode', 'authorization': [{'account': 'currency', 'permission': 'active'}], 'data': '00000079b822651d0100e5096966205f5f6e616d655f5f203d3d20275f5f6d61696e5f5f273a0a09696d706f727420656f736c69625f64756d6d7920617320656f736c69620a656c73653a0a09696d706f727420656f736c69620a696d706f7274207374727563740a0a636f6465203d20656f736c69622e4e28622763757272656e637927290a7461626c65203d20656f736c69622e4e2862276163636f756e7427290a0a636c617373204163636f756e74286f626a656374293a0a096b6579203d20656f736c69622e4e2862276163636f756e7427290a09646566205f5f696e69745f5f2873656c662c73636f70652c62616c616e63653d30293a0a090973656c662e73636f7065203d2073636f70650a090969662062616c616e6365203d3d20303a0a09090973656c662e6c6f616428290a0909656c73653a0a09090973656c662e62616c616e6365203d2062616c616e63650a09646566206973456d7074792873656c66293a0a090972657475726e2062616c616e6365203d3d20300a096465662073746f72652873656c66293a0a0909656f736c69622e73746f72655f7536342873656c662e73636f70652c636f64652c7461626c652c4163636f756e742e6b65792c73656c662e62616c616e6365290a09646566206c6f61642873656c66293a0a090973656c662e62616c616e6365203d20656f736c69622e6c6f61645f7536342873656c662e73636f70652c636f64652c7461626c652c4163636f756e742e6b6579290a0a64656620696e697428293a0a23097072696e74282768656c6c6f2066726f6d20696e697427290a0961203d204163636f756e7428636f64652c313030303030290a09612e73746f726528290a0a646566206170706c79286e616d652c74797065293a0a23097072696e74282768656c6c6f2066726f6d20707974686f6e206170706c79272c6e616d652c74797065290a23097072696e7428656f736c69622e6e3273286e616d65292c656f736c69622e6e3273287479706529290a0969662074797065203d3d20656f736c69622e4e2862277472616e7366657227293a0a09096d7367203d20656f736c69622e726561644d65737361676528290a0909726573756c74203d207374727563742e756e7061636b2827515151272c6d7367290a2309097072696e7428726573756c74290a090966726f6d5f203d20726573756c745b305d0a0909746f5f203d20726573756c745b315d0a0909616d6f756e74203d20726573756c745b325d0a09090a0909656f736c69622e7265717569726541757468282066726f6d5f20293b0a0909656f736c69622e726571756972654e6f74696365282066726f6d5f20293b0a0909656f736c69622e726571756972654e6f746963652820746f5f20290a0a090966726f6d5f203d204163636f756e742866726f6d5f290a0909746f5f203d204163636f756e7428746f5f290a090969662066726f6d5f2e62616c616e6365203e3d20616d6f756e743a0a09090966726f6d5f2e62616c616e6365202d3d20616d6f756e740a090909746f5f2e62616c616e6365202b3d20616d6f756e740a09090966726f6d5f2e73746f726528290a090909746f5f2e73746f726528290a0a6966205f5f6e616d655f5f203d3d20275f5f6d61696e5f5f273a0a09696e697428290a096170706c7928656f736c69622e4e286227707974686f6e27292c656f736c69622e4e2862277472616e736665722729290a0a0a0a010b4163636f756e744e616d65044e616d6502087472616e7366657200030466726f6d0b4163636f756e744e616d6502746f0b4163636f756e744e616d6506616d6f756e740655496e743634076163636f756e740002076163636f756e74044e616d650762616c616e63650655496e74363401000000b298e982a4087472616e736665720100000080bafac6080369363401076163636f756e7400076163636f756e74'}], 'output': [{'notify': [], 'deferred_transactions': []}]}}
```
### >>> r = eosapi.get_account('currency')
### >>> r
```
{'name': 'currency', 'balance': 0, 'stakedBalance': 1, 'unstakingBalance': 0, 'lastUnstakingTime': '2106-02-07T06:28:15', 'abi': '{"types":[{"newTypeName":"AccountName","type":"Name"}],"structs":[{"name":"transfer","base":"","fields":{"from":"AccountName","to":"AccountName","amount":"UInt64"}},{"name":"account","base":"","fields":{"account":"Name","balance":"UInt64"}}],"actions":[{"action":"transfer","type":"transfer"}],"tables":[{"table":"account","indextype":"i64","keynames":["account"],"keytype":[],"type":"account"}]}'}
```
### >>> r.balance
```
0
```
### >>> eosapi.get_table('currency','currency','account')
```
{'rows': [{'account': 'account', 'balance': 100000}], 'more': False}
```
### >>> r = eosapi.get_table('currency','currency','account')
### >>> r.rows[0].balance
```
100000
```

### >>> r = eosapi.push_message('currency','transfer','{"from":"currency","to":"inita","amount":50}',['currency','inita'],{'currency':'active'},False)
### >>> eosapi.get_table('currency','currency','account')
```
{'rows': [{'account': 'account', 'balance': 99950}], 'more': False}
```
### >>> eosapi.get_table('inita','currency','account')
```
{'rows': [{'account': 'account', 'balance': 50}], 'more': False}
```

# wallet api

### >>> import wallet
### >>> wallet_name = 'mywallet'
### >>> password = wallet.create(wallet_name) 
```
2439175ms thread-1   wallet.cpp:182                save_wallet_file     ] saving wallet to file /Users/newworld/dev/eos/build/programs/data-dir/./mywallet.wallet
```
### >>> print(password)
```
b'PW5JGYdyzzZYTQpPNFU3bb83fQW3P8LUUQ5w2D8GL8zaB93riScSE'
```
### >>> wallet.open(wallet_name) 
```
True
```
### >>> wallet.unlock(wallet_name,password)
```
True
```
### >>> wallet.import_key(wallet_name,'5JaXKGt2J4kK4EabnPZ2DSS6UJFe8vZ9mgEBXDQ5j3G2m7HE5nQ')
```
2439178ms thread-1   wallet.cpp:182                save_wallet_file     ] saving wallet to file /Users/newworld/dev/eos/build/programs/data-dir/./mywallet.wallet
True
```
### >>> wallet.list_keys()
```
{b'EOS6V8Ho3VBuxNrGsqHWVXHfRPThPhqFDs9aPp1s9hJqAKZJ7NTJJ': b'5JaXKGt2J4kK4EabnPZ2DSS6UJFe8vZ9mgEBXDQ5j3G2m7HE5nQ'}
```
### >>> wallet.list_wallets()
```
[b'mywallet *']
```
### >>> eosapi.create_key()
```
(b'EOS7VMTHQiCKzynXZHvW2Ef1zi8w92G6ayM7bdh1kV6dgewyBB8i7', b'5HpsDntjXznKoFb13QVBeAD6FUE7hwRjCdGrAs3Wq8GxqK9m8Qs')
### >>> wallet.import_key(wallet_name,b'5HpsDntjXznKoFb13QVBeAD6FUE7hwRjCdGrAs3Wq8GxqK9m8Qs')
3519376ms thread-1   wallet.cpp:182                save_wallet_file     ] saving wallet to file /Users/newworld/dev/eos/build/programs/data-dir/./mywallet.wallet
True
```
### >>> wallet.list_keys()
```
{b'EOS6V8Ho3VBuxNrGsqHWVXHfRPThPhqFDs9aPp1s9hJqAKZJ7NTJJ': b'5JaXKGt2J4kK4EabnPZ2DSS6UJFe8vZ9mgEBXDQ5j3G2m7HE5nQ', b'EOS7VMTHQiCKzynXZHvW2Ef1zi8w92G6ayM7bdh1kV6dgewyBB8i7': b'5HpsDntjXznKoFb13QVBeAD6FUE7hwRjCdGrAs3Wq8GxqK9m8Qs'}
```
