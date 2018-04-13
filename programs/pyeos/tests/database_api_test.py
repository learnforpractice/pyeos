import database_api
n = eosapi.N('hello')
itr = database_api.find_i64(n, n, n, eosapi.N('name'))
ret = database_api.get_i64(itr)
print(ret)


