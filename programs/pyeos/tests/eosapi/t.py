def tf(name='mike'):
    msg = {"from":"eosio", "to":"hello", "quantity":"100.0000 EOS", "memo":"m"}
    act = ['eosio.token','transfer', msg, {'eosio':'active'}]
    r = eosapi.push_transactions([[act]], True, 0, False, False, 0x7fffffff)
    print(r)
