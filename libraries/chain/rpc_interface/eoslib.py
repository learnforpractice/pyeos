import database_api

client = None

def char_to_symbol(c):
    c = ord(c)
    a = ord('a')
    z = ord('z')
    _1 = ord('1')
    _5 = ord('5')

    if c >= a and c <= z:
        return (c - a) + 6
    
    if c >= _1 and c <= _5:
        return (c - _1) + 1
    return 0

def N(name):
    _len = len(name);
    value = 0;
    
    for i in range(13):
        c = 0
        if i < _len and i <= 12:
            c = char_to_symbol(name[i])

        if i < 12:
            c &= 0x1f;
            c <<= 64-5*(i+1)
        else:
            c &= 0x0f;
        value |= c;
    
    return value

def set_client(_client):
    global client
    client = _client

def get_client():
    global client
    return client

def read_action():
    return client.read_action()

def db_store_i64(scope: int, table: int, payer: int, id: int, buffer: bytes):
    return client.db_store_i64(scope, table, payer, id, buffer)

def db_update_i64(itr: int, payer: int, buffer: bytes):
    client.db_update_i64(itr, payer, buffer)

def db_remove_i64(itr: int):
    client.db_remove_i64(itr) 

def db_get_i64(itr: int):
#    return database_api.get_i64(itr)
    return client.db_get_i64(itr)

def db_next_i64(itr: int):
#    return database_api.next_i64(itr)
    return client.db_next_i64(itr)

def db_previous_i64(itr: int):
#    return database_api.previous_i64(itr)
    return client.db_previous_i64(itr)

def db_find_i64(code: int, scope: int, table: int, id: int):
#    return database_api.find_i64(code, scope, table, id)
    return client.db_find_i64(code, scope, table, id)

def db_lowerbound_i64(code: int, scope: int, table: int, id: int):
    return database_api.lowerbound_i64(code, scope, table, id)

def db_upperbound_i64(code: int, scope: int, table: int, id: int):
    return database_api.upperbound_i64(code, scope, table, id)

def db_end_i64(code: int, scope: int, table: int):
    return database_api.end_i64(code, scope, table)