import eoslib
n = eosapi.N('rpctest')
itr = eoslib.db_find_i64(n, n, n, eosapi.N('name'))
ret = eoslib.db_get_i64(itr)
print(ret)


