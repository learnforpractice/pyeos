from eoslib import *
import ustruct as struct

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
scope = N('apitest')
code = N('apitest')
table = N('apitest')
payer = N('apitest')

def db_test_i64():
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
        if itr == -1:
            break
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
        eosio_assert(value.decode('utf8') == 'hello:'+str(id), 'db_get_i64 failed')

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

'''
int db_idx64_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* secondary , size_t len); \
void db_idx64_update( int iterator, uint64_t payer, const char* secondary , size_t len ); \
void db_idx64_remove( int iterator ); \
int db_idx64_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const char* secondary , size_t len, uint64_t* primary ); \
int db_idx64_find_primary( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t primary ); \
int db_idx64_lowerbound( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t* primary ); \
int db_idx64_upperbound( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t* primary ); \
int db_idx64_end( uint64_t code, uint64_t scope, uint64_t table ); \
int db_idx64_next( int iterator, uint64_t* primary  ); \
int db_idx64_previous( int iterator, uint64_t* primary );
'''

def db_test_idx64():
    table = N('myindextable')
    receiver = code
    records = [[265, N('alice')],
               [781, N('bob')],
               [234, N('charlie')],
               [650, N('allyson')],
               [540, N('bob')],
               [976, N('emily')],
               [110, N('joe')]
               ];

    records = [[265, 1],
               [781, 2],
               [234, 3],
               [650, 4],
               [540, 5],
               [976, 6],
               [110, 7]
               ];

    for i in range(len(records)):
        id = records[i][0]
        secondary = struct.pack('Q', records[i][1])
        itr, _ = db_idx64_find_primary(receiver, receiver, table, id)
        print('db_idx64_find_primary:', itr, _)
        if itr < 0:
            db_idx64_store(receiver, table, receiver, id, secondary)
        else:
            db_idx64_update(itr, receiver, secondary)

    print('-------------db_idx64_end---------------')
    itr = db_idx64_end(receiver, receiver, table)
    print('db_idx64_end', itr)
    while itr != -1:
        itr, primary = db_idx64_previous(itr)
        if itr < 0:
            break
#        print('db_idx64_previous', itr, primary)
        itr_primary, secondary = db_idx64_find_primary(receiver, receiver, table, primary)
        secondary, = struct.unpack('Q', secondary)
        print(primary, secondary)

    #find_primary
    sec = 0;
    itr, sec = db_idx64_find_primary(receiver, receiver, table, 999)
    sec, = struct.unpack('Q', sec)
    print('db_idx64_find_primary', itr, sec)
    eosio_assert(itr < 0 and sec == 0, "idx64_general - db_idx64_find_primary")

    itr, sec = db_idx64_find_primary(receiver, receiver, table, 110)
    print('db_idx64_find_primary', itr, sec)
    sec, = struct.unpack('Q', sec)
    eosio_assert(itr >= 0, "idx64_general - db_idx64_find_primary")

    print('------------------------')
    itr, sec = db_idx64_find_primary(receiver, receiver, table, 781)
    print('db_idx64_find_primary', itr, sec)

    itr_next = itr
    while True:
        itr_next, prim_next = db_idx64_next(itr_next)
        if itr_next == -1:
            break
        print('db_idx64_next', itr_next, prim_next)

        itr, sec = db_idx64_find_primary(receiver, receiver, table, prim_next)
        print('db_idx64_find_primary', itr, sec)

    secondary = struct.pack('Q', N('joe'))
    itr, primary = db_idx64_find_secondary(receiver, receiver, table, secondary)
    print('db_idx64_find_secondary', itr, primary)

def db_test_idx256():
    table = N('myindextable')
    receiver = code
    records = [[265, N('alice')],
               [781, N('bob')],
               [234, N('charlie')],
               [650, N('allyson')],
               [540, N('bob')],
               [976, N('emily')],
               [110, N('joe')]
               ];
    if 0:
        for i in range(len(records)):
            id = records[i][0]
            secondary = struct.pack('QQQQ', records[i][1], 1, 1, 1)
            ret = db_idx256_store(receiver, table, receiver, id, secondary)
            print('idx256_store:', ret)

    #find_primary
    sec = 0;
    itr, sec = db_idx256_find_primary(receiver, receiver, table, 999)
    print('db_idx256_find_primary', itr, sec)
    eosio_assert(itr < 0 , "idx256_general - db_idx256_find_primary")

    itr, sec = db_idx256_find_primary(receiver, receiver, table, 110)
    print('db_idx256_find_primary', itr, sec)
    eosio_assert(itr >= 0, "idx256_general - db_idx256_find_primary")

    itr_next, prim_next = db_idx256_next(itr)
    eosio_assert(itr_next < 0 and prim_next == 0, "idx256_general - db_idx256_find_primary")

    itr, sec = db_idx256_find_primary(receiver, receiver, table, 110)
    
    secondary = struct.pack('QQQQ', N('joe'), 1, 1, 1)
    itr, primary = db_idx256_find_secondary(receiver, receiver, table, secondary)
    print('db_idx256_find_secondary', itr, primary)

def inline_send():
    account = N('hello')
    auth = struct.pack('QQQQ', code, N('active'), account, N('active'))
    send_inline(account, N('sayhello'), auth, b'hello,worldddddddd')

'''
struct mp_transaction {
   struct mp_action*      context_free_actions;
   size_t                 free_actions_len;
   struct mp_action*      actions;
   size_t                 actions_len;

   uint32_t               expiration;   ///< the time at which a transaction expires
   uint16_t               region              ; ///< the computational memory region this transaction applies to.
   uint16_t               ref_block_num       ; ///< specifies a block num in the last 2^16 blocks.
   uint32_t               ref_block_prefix    ; ///< specifies the lower 32 bits of the blockid at get_ref_blocknum
   uint32_t               max_net_usage_words ; /// upper limit on total network bandwidth (in 8 byte words) billed for this transaction
   uint32_t               max_kcpu_usage      ; /// upper limit on the total number of kilo CPU usage units billed for this transaction
   uint32_t               delay_sec           ; /// number of seconds to delay this transaction for during which it may be canceled.
};
'''

def deffer_send():
    print('++++++++deffer_send')
    sender_id = struct.pack('QQ', 123)
    account = code
    auth = struct.pack('QQ', account, N('active'))
    free_actions = [ [account, N('sayhello'), None, b'hello,world  ddddd'] ]
    actions = [ [account, N('sayhello'), auth, b'hello,worldddddd   d'] ]
    ref_block = 0
    ref_block_prefix = 0
    send_deferred(sender_id, payer, now()+5, 0, ref_block, ref_block_prefix, 100000, 100000, 2 , free_actions, actions);

def apply(name, type):
#    eosio_delay(190)
    if type == N('dbtest'):
#        db_test()
        db_test_i64()
#        db_test_idx64()
#        db_test_idx256()
#        deffer_send()
    elif type == N('deffersend'):
        deffer_send()
    elif type == N('inlinesend'):
        print('inline_send begin')
        inline_send()
        print('inline_send return')
    elif type == N('sayhello'):
        act = read_action()
        print(n2s(name), 'read_action return:', act)
#        deffer_send()
    elif type == N('callwasm'):
        print('hello,world')
        wasm_call(N('lab'), 'sayHello')
    print('')
    
    
    