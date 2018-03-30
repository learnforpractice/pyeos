import ustruct
from eoslib import *

g_scope = N('storage')
g_code = N('storage')
payer = N('storage')

class storage_list(object):
    pass

# key_type key_length value_type value_length key_data value_data
class storage_dict(object):
    def __init__(self,table_id):
        self._dict = {}
        self.table_id = table_id

        itr = db_end_i64(g_code, g_scope, table_id)
        if itr == -1: #no value in table
            return
        print('+++itr:', itr) # itr should be -2
        while True:
            itr, key = db_previous_i64(itr)
            if itr < 0:
                break
            value = db_get_i64(itr)

            key_type, key_length, value_type, value_length = ustruct.unpack('BHBH', value)

            _value = None
            if key_type == 0: #int
                _key = int.from_bytes(value[8:8+key_length], 'little')
            elif key_type == 1: #str
                _key = value[8:8+key_length:]
            elif key_type == 2: #list
                table_id = int.from_bytes(value[8:8+key_length], 'little')
                _key = storage_list(table_id)
            elif key_type == 3:#dict
                table_id = int.from_bytes(value[8:8+key_length], 'little')
                _key = storage_dict(table_id)
            else:
                raise Exception('unknown key type')

            if value_type == 0: #int
                _value = int.from_bytes(value[8+key_length:], 'little')
            elif value_type == 1: #str
                _value = value[8+key_length:]
            elif value_type == 2: #list
                table_id = int.from_bytes(value[8+key_length:], 'little')
                _value = storage_list(table_id)
            elif value_type == 3:#dict
                table_id = int.from_bytes(value[8+key_length:], 'little')
                _value = storage_dict(table_id)
            else:
                raise Exception('unknown key type')

            self._dict[_key] = _value

    def get_hash(self, v):
        if type(v) is int:
            return v
        elif type(v) in (str, bytes):
            return hash(v)
        elif type(v) in (storage_dict, storage_list):
            return v.table_id
        else:
            raise Exception('unsupported value type')

    def get_type(self,val):
        if type(val) == int:
            return 0
        elif type(val) == str:
            return 1
        elif type(val) == storage_list:
            return 2
        elif type(val) == storage_dict:
            return 3
        else:
            raise Exception('unsupported type')

    def get_raw_data(self, data):
        data_type = self.get_type(data)
        raw_data = 0
        raw_length = 0
        if data_type == 0: #int
            raw_length = 8
            raw_data = int.to_bytes(data, 8, 'little')
        elif data_type == 1: #str
            raw_length = len(data)
            raw_data = data
        elif data_type == 2: #list
            raw_length = 8
            raw_data = data.table_id
        elif data_type == 3: #dict
            raw_length = 8
            raw_data = data.table_id
        return (data_type, raw_length, raw_data)
    
    def __getitem__(self, key):
        return self._dict[key]

    def __setitem__(self, key, val):
        if key in self._dict and val == self._dict[key]:
            return

        id = self.get_hash(key)

        key_type, raw_key_length, raw_key_data = self.get_raw_data(key)
        value_type, raw_value_length, raw_value_data = self.get_raw_data(val)

        _value = ustruct.pack('BHBH', key_type, raw_key_length, value_type, raw_value_length)

        _value += raw_key_data
        _value += raw_value_data
        itr = db_find_i64(g_code, g_scope, self.table_id, id)
        if itr < 0:
            db_store_i64(g_scope, self.table_id, payer, id, _value)
        else:
            db_update_i64(itr, payer, _value)

    def __iter__(self):
        return iter(self._dict)
    
    def __len__(self):
        return len(self._dict)

    def __delitem__(self, key):
        id = self.get_hash(key)
        del self._dict[key]
        itr = db_find_i64(g_code, g_scope, self.table_id, key)
        if itr >= 0:
            db_remove_i64(itr)

    def __repr__(self):
        dictrepr = dict.__repr__(self)
        return '%s(%s)' % (type(self).__name__, dictrepr)

def apply(name, type):
    require_auth(g_code)
    a = storage_dict(123)
    for key in a:
        print(key, a[key])
    a[100] = 'hello'
    a[101] = 'world'
    a['name'] = 'mike'
    msg = read_action()
    a[msg] = msg
    if 101 in a:
        del a[101]

