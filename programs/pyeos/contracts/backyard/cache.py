import ustruct
from eoslib import *
import db

g_scope = N('cache')
g_code = N('cache')
payer = N('cache')

class CList(object):
    def __init__(self,table_id):
        self._list = []
        self._dirty_keys = {}
        self.table_id = N('list'+str(table_id))

    def load(self):
        itr = db.end_i64(g_code, g_scope, self.table_id)
        if itr == -1: #no value in table
            return
        while True:
            itr, key = db.previous_i64(itr)
            if itr < 0:
                break
            value = db.get_i64(itr)
            value_type = ustruct.unpack('B', value)
            _value = None

            if value_type == 0: #int
                _value = int.from_bytes(value[2:], 'little')
            elif value_type == 1: #str
                _value = value[2:]
            else:
                raise TypeError('unknown key type')

            self._dict[_key] = _value

    def update(self, key, val):
        id = key
        value_type, raw_value_length, raw_value_data = self.get_raw_data(val)

        _value = ustruct.pack('B', value_type)
        _value += raw_value_data
        
        itr = db.find_i64(g_code, g_scope, self.table_id, id)
        if itr < 0:
            db.store_i64(g_scope, self.table_id, payer, id, _value)
        else:
            db.update_i64(itr, payer, _value)

    def store(self):
        for key in self._dirty_keys:
            self.update(key, self._list[key])

    def get_hash(self, v):
        if type(v) is int:
            return v
        elif type(v) in (str, bytes):
            return hash64(v)
        elif type(v) in (storage_dict, storage_list):
            return v.table_id
        else:
            raise TypeError('unsupported value type')

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
            raise TypeError('unsupported type')

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
        return (data_type, raw_length, raw_data)

    def __getitem__(self, index):
        return self._list[index]

    def __setitem__(self, index, val):
        if index < len(self._list) and self._list[index] == val:
            return
        else:
            self._list[index] = val
            self._dirty_keys[index] = True

    def __iter__(self):
        return iter(self._list)
    
    def __len__(self):
        return len(self._list)

    def __delitem__(self, index):
        id = index
        del self._list[key]
        del self._dirty_keys[index]
        itr = db.find_i64(g_code, g_scope, self.table_id, id)
        if itr >= 0:
            db.remove_i64(itr)

    def __repr__(self):
        return '%s(%s)' % (type(self).__name__, str(self._list))

# key_type key_length value_type value_length key_data value_data
class cache_dict(object):
    def __init__(self,table_id):
        self._dict = {}
        self._dirty_keys = {}
        self.table_id = N('cache'+str(table_id))

    def load(self):
        itr = db.end_i64(g_code, g_scope, self.table_id);
        if itr == -1: #no value in table
            return
        while True:
            itr, key = db.previous_i64(itr)
            if itr < 0:
                break
            value = db.get_i64(itr)

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
                raise TypeError('unknown key type')

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
                raise TypeError('unknown key type')

            self._dict[_key] = _value

    def update(self, key, val):
        id = self.get_hash(key)

        key_type, raw_key_length, raw_key_data = self.get_raw_data(key)
        value_type, raw_value_length, raw_value_data = self.get_raw_data(val)

        _value = ustruct.pack('BHBH', key_type, raw_key_length, value_type, raw_value_length)

        _value += raw_key_data
        _value += raw_value_data
        itr = db.find_i64(g_code, g_scope, self.table_id, id)
        if itr < 0:
            db.store_i64(g_scope, self.table_id, payer, id, _value)
        else:
            db.update_i64(itr, payer, _value)

    def store(self):
        for key in self._dirty_keys:
            self.update(key, self._dict[key])

    def get_hash(self, v):
        if type(v) is int:
            return v
        elif type(v) in (str, bytes):
            return hash64(v)
        elif type(v) in (storage_dict, storage_list):
            return v.table_id
        else:
            raise TypeError('unsupported value type')

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
            raise TypeError('unsupported type')

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
            raw_data = int.to_bytes(data.table_id, 8, 'little')
        elif data_type == 3: #dict
            raw_length = 8
            raw_data = int.to_bytes(data.table_id, 8, 'little')
        return (data_type, raw_length, raw_data)

    def __getitem__(self, key):
        return self._dict[key]

    def __setitem__(self, key, val):
        if key in self._dict and self._dict[key] == val:
            return
        else:
            self._dict[key] = val
            self._dirty_keys[key] = True

    def __iter__(self):
        return iter(self._dict)
    
    def __len__(self):
        return len(self._dict)

    def __delitem__(self, key):
        id = self.get_hash(key)
        del self._dict[key]
        del self._dirty_keys[key]
        itr = db.find_i64(g_code, g_scope, self.table_id, id)
        if itr >= 0:
            db.remove_i64(itr)

    def __repr__(self):
        return '%s(%s)' % (type(self).__name__, str(self._dict))

def apply(receiver, name, type):
    require_auth(g_code)
    a = cache(123)
    a.load()
    print('-----')
    for key in a:
        print(key, a[key])
    a[100] = 'hello'
    a[101] = 'world'
    a['name'] = 'mike'
    if 101 in a:
        del a[101]
    msg = read_action()
    a[msg] = msg
    a.store()
    print('+++++++++')
    for key in a:
        print(key, a[key])


