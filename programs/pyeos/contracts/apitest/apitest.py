from eoslib import *
'''
int db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size );
void db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size );
void db_remove_i64( int itr );
int db_get_i64( int itr, char* buffer, size_t buffer_size );
int db_next_i64( int itr, uint64_t* primary );
int db_previous_i64( int itr, uint64_t* primary );
int db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_end_i64( uint64_t code, uint64_t scope, uint64_t table );
'''
scope = N('test')
code = N('test')
table = N('test')
payer = N('test')

def db_test():
    count = 100
    error = None
#    keys = [n for n in range(count)]
    keys = [3, 8, 7, 100, 59, 120, 300]
    try:
        db_store_i64(scope, table, payer, 'abc', 'def')
    except Exception as e:
        print(e)
        error = e
    eosio_assert(error != None, 'db_store_i64 should raise type convertion exception!')

    error = None
    try:
        db_store_i64(scope, table, payer, 123, [1,2,3])
    except Exception as e:
        print('+++++',e)
        error = e
    eosio_assert(error != None, 'db_store_i64 should raise type convertion exception!')

    while True:
        itr = db_end_i64(code, scope, table)
#        print('++++++itr:',itr)
        itr, key = db_previous_i64(itr)
#        print('+++', itr, key)
        if itr < 0:
            break
        db_remove_i64(itr)


    print('================test db_store_i64============')
    for id in keys:
        itr = db_store_i64(scope, table, payer, id, 'hello:'+str(id))
        eosio_assert(itr >= 0, 'store failed')

    print('=================test db_find_i64============')
    for id in keys:
        itr = db_find_i64(scope, table, payer, id)
        eosio_assert(itr >= 0, 'db_find_i64')
        value = db_get_i64(itr)
        eosio_assert(value == 'hello:'+str(id), 'db_get_i64 failed')

    print('=================test db_previous_i64============')
    itr = db_end_i64(code, scope, table)
#    print('++++++itr:',itr)

    keys.sort()
    keys.reverse()
    for id in keys:
        itr, key = db_previous_i64(itr)
        print('+++', itr, key, id)
        eosio_assert(key == id, 'db_previous_i64 failed')
        value = db_get_i64(itr)
        print(key, value)

    keys.reverse()


    print('=================test db_lowerbound_i64============')
    itr = db_lowerbound_i64( code, scope, table, 199 )
    print('+++++', itr)
    value = db_get_i64(itr)
    print(itr, value)

    print('=================test db_upperbound_i64============')
    itr = db_upperbound_i64( code, scope, table, 2 )
    print('+++++', itr)
    value = db_get_i64(itr)
    print(itr, 3, value)

    print('=================test db_update_i64============')
    for id in keys:
        itr = db_find_i64(scope, table, payer, id)
        eosio_assert(itr >= 0, 'db_find_i64')
        db_update_i64(itr, payer, 'world' + str(id))

    print('=================end test============')
    while True:
        itr = db_end_i64(code, scope, table)
        itr, key = db_previous_i64(itr)
#        print('+++', itr, key)
        if itr < 0:
            break
        db_remove_i64(itr)

def apply(name, type):
    if type == N('dbtest'):
        db_test()
        
        
    