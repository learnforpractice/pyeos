Hack on Eos. Have fun!

$ ./pyeos/pyeos --skip-transaction-signatures

$ ./pyeos/pyeos --skip-transaction-signatures

# 1. blockchain api test

### >>> import eosapi

### >>> info = eosapi.get_info()

### >>> info


info = eosapi.get_info() json_str.size():336 info {'head_block_num': 2110, 'last_irreversible_block_num': 2092, 'head_block_id': '0000083ef40927dffaee69ff02a80beb7b21a2238e91300ac25d259179d7830d', 'head_block_time': '2017-08-27T09:22:00', 'head_block_producer': 'initp', 'recent_slots': '1111111111111111111111111111111111111111111111111111111111111111', 'participation_rate': '1.00000000000000000'}
{'head_block_num': 2110, 'last_irreversible_block_num': 2092, 'head_block_id': '0000083ef40927dffaee69ff02a80beb7b21a2238e91300ac25d259179d7830d', 'head_block_time': '2017-08-27T09:22:00', 'head_block_producer': 'initp', 'recent_slots': '1111111111111111111111111111111111111111111111111111111111111111', 'participation_rate': '1.00000000000000000'}
### >>> key = eosapi.create_key()

public: EOS5XnfJ2ahqxhmZz47mc1nceJdsumB4Mtpwu3tDDnnj9Pqobzn6o private: 5Ka18FuiqDTGHi6hRqqYRHeC8iabMzjLSbNegpvL9rosmRfC8qV

### >>> print(key)

(b'EOS5XnfJ2ahqxhmZz47mc1nceJdsumB4Mtpwu3tDDnnj9Pqobzn6o', b'5Ka18FuiqDTGHi6hRqqYRHeC8iabMzjLSbNegpvL9rosmRfC8qV')

### >>> key1 = 'EOS4toFS3YXEQCkuuw1aqDLrtHim86Gz9u3hBdcBw5KNPZcursVHq'

### >>> key2 = 'EOS6KdkmwhPyc2wxN9SAFwo2PU2h74nWs7urN1uRduAwkcns2uXsa'

### >>> eosapi.create_account('inita','tester5',key1,key2)


key = eosapi.create_key() public: EOS5XnfJ2ahqxhmZz47mc1nceJdsumB4Mtpwu3tDDnnj9Pqobzn6o private: 5Ka18FuiqDTGHi6hRqqYRHeC8iabMzjLSbNegpvL9rosmRfC8qV print(key) (b'EOS5XnfJ2ahqxhmZz47mc1nceJdsumB4Mtpwu3tDDnnj9Pqobzn6o', b'5Ka18FuiqDTGHi6hRqqYRHeC8iabMzjLSbNegpvL9rosmRfC8qV') key1 = 'EOS4toFS3YXEQCkuuw1aqDLrtHim86Gz9u3hBdcBw5KNPZcursVHq' key2 = 'EOS6KdkmwhPyc2wxN9SAFwo2PU2h74nWs7urN1uRduAwkcns2uXsa' eosapi.create_account('inita','tester5',key1,key2) { "transaction_id": "687d06e777bd8deedca0a7a92938b6fd44e85599d7ab8d351f4e98b42e8be082", "processed": { "refBlockNum": 2033, "refBlockPrefix": 2586037830, "expiration": "2017-08-27T09:19:49", "scope": [ "eos", "inita" ], "signatures": [], "messages": [{ "code": "eos", "type": "newaccount", "authorization": [{ "account": "inita", "permission": "active" } ], "data": "000000008040934b000000e0cb4267a101000000010200b35ad060d629717bd3dbec82731094dae9cd7e9980c39625ad58fa7f9b654b010000010000000102bcca6347d828d4e1868b7dfa91692a16d5b20d0ee3d16a7ca2ddcc7f6dd03344010000010000000001000000008040934b00000000149be8080100010000000000000008454f5300000000" } ], "output": [{ "notify": [], "sync_transactions": [], "async_transactions": [] } ] } }
{ "transaction_id": "687d06e777bd8deedca0a7a92938b6fd44e85599d7ab8d351f4e98b42e8be082", "processed": { "refBlockNum": 2033, "refBlockPrefix": 2586037830, "expiration": "2017-08-27T09:19:49", "scope": [ "eos", "inita" ], "signatures": [], "messages": [{ "code": "eos", "type": "newaccount", "authorization": [{ "account": "inita", "permission": "active" } ], "data": "000000008040934b000000e0cb4267a101000000010200b35ad060d629717bd3dbec82731094dae9cd7e9980c39625ad58fa7f9b654b010000010000000102bcca6347d828d4e1868b7dfa91692a16d5b20d0ee3d16a7ca2ddcc7f6dd03344010000010000000001000000008040934b00000000149be8080100010000000000000008454f5300000000" } ], "output": [{ "notify": [], "sync_transactions": [], "async_transactions": [] } ] } }
### >>> r = eosapi.get_transaction('687d06e777bd8deedca0a7a92938b6fd44e85599d7ab8d351f4e98b42e8be082')

### >>> print(r)


r = eosapi.get_transaction('687d06e777bd8deedca0a7a92938b6fd44e85599d7ab8d351f4e98b42e8be082') print(r) {'transaction': {'refBlockNum': 2033, 'refBlockPrefix': 2586037830, 'expiration': '2017-08-27T09:19:49', 'scope': ['eos', 'inita'], 'signatures': [], 'messages': [{'code': 'eos', 'type': 'newaccount', 'authorization': [{'account': 'inita', 'permission': 'active'}], 'data': '000000008040934b000000e0cb4267a101000000010200b35ad060d629717bd3dbec82731094dae9cd7e9980c39625ad58fa7f9b654b010000010000000102bcca6347d828d4e1868b7dfa91692a16d5b20d0ee3d16a7ca2ddcc7f6dd03344010000010000000001000000008040934b00000000149be8080100010000000000000008454f5300000000'}], 'output': [{'notify': [], 'sync_transactions': [], 'async_transactions': []}]}}
{'transaction': {'refBlockNum': 2033, 'refBlockPrefix': 2586037830, 'expiration': '2017-08-27T09:19:49', 'scope': ['eos', 'inita'], 'signatures': [], 'messages': [{'code': 'eos', 'type': 'newaccount', 'authorization': [{'account': 'inita', 'permission': 'active'}], 'data': '000000008040934b000000e0cb4267a101000000010200b35ad060d629717bd3dbec82731094dae9cd7e9980c39625ad58fa7f9b654b010000010000000102bcca6347d828d4e1868b7dfa91692a16d5b20d0ee3d16a7ca2ddcc7f6dd03344010000010000000001000000008040934b00000000149be8080100010000000000000008454f5300000000'}], 'output': [{'notify': [], 'sync_transactions': [], 'async_transactions': []}]}}


# 2. wallet test

### >>> import wallet
### >>> wallet_name = 'mywallet'
### >>> password = wallet.create(wallet_name) 
2439175ms thread-1   wallet.cpp:182                save_wallet_file     ] saving wallet to file /Users/newworld/dev/eos/build/programs/data-dir/./mywallet.wallet
### >>> print(password)
b'PW5JGYdyzzZYTQpPNFU3bb83fQW3P8LUUQ5w2D8GL8zaB93riScSE'
### >>> wallet.open(wallet_name) 
True
### >>> wallet.unlock(wallet_name,password)
True
### >>> wallet.import_key(wallet_name,'5JaXKGt2J4kK4EabnPZ2DSS6UJFe8vZ9mgEBXDQ5j3G2m7HE5nQ')
2439178ms thread-1   wallet.cpp:182                save_wallet_file     ] saving wallet to file /Users/newworld/dev/eos/build/programs/data-dir/./mywallet.wallet
True
### >>> wallet.list_keys()
{b'EOS6V8Ho3VBuxNrGsqHWVXHfRPThPhqFDs9aPp1s9hJqAKZJ7NTJJ': b'5JaXKGt2J4kK4EabnPZ2DSS6UJFe8vZ9mgEBXDQ5j3G2m7HE5nQ'}
### >>> wallet.list_wallets()
[b'mywallet *']

### >>> eosapi.create_key()
(b'EOS7VMTHQiCKzynXZHvW2Ef1zi8w92G6ayM7bdh1kV6dgewyBB8i7', b'5HpsDntjXznKoFb13QVBeAD6FUE7hwRjCdGrAs3Wq8GxqK9m8Qs')
### >>> wallet.import_key(wallet_name,b'5HpsDntjXznKoFb13QVBeAD6FUE7hwRjCdGrAs3Wq8GxqK9m8Qs')
3519376ms thread-1   wallet.cpp:182                save_wallet_file     ] saving wallet to file /Users/newworld/dev/eos/build/programs/data-dir/./mywallet.wallet
True

### >>> wallet.list_keys()
{b'EOS6V8Ho3VBuxNrGsqHWVXHfRPThPhqFDs9aPp1s9hJqAKZJ7NTJJ': b'5JaXKGt2J4kK4EabnPZ2DSS6UJFe8vZ9mgEBXDQ5j3G2m7HE5nQ', b'EOS7VMTHQiCKzynXZHvW2Ef1zi8w92G6ayM7bdh1kV6dgewyBB8i7': b'5HpsDntjXznKoFb13QVBeAD6FUE7hwRjCdGrAs3Wq8GxqK9m8Qs'}

