import ustruct
from eoslib import *

g_scope = N('storage')
g_code = N('storage')
payer = N('storage')

class SList(object):
    #FIXME
    table_id_counter = 0
    def __init__(self,table_id = None, object_type = None):
        self._list = []
        if table_id:
            self.table_id = table_id
        else:
            SList.table_id_counter += 1
            table_id = SList.table_id_counter

        self.object_type = object_type
        
        itr = db_end_i64(g_code, g_scope, table_id)
        if itr == -1: #no value in table
            return
        print('+++itr:', itr) # itr should be -2
        while True:
            itr, key = db_previous_i64(itr)
            if itr < 0:
                break
            value = db_get_i64(itr)
            if object_type:
                _obj = object_type.unpack(value)
                self._list.insert(0, _obj)
                continue

            value_type = ustruct.unpack('B', value)

            if value_type == 0: #int
                _value = int.from_bytes(value[2:], 'little')
            elif value_type == 1: #str
                _value = value[2:]
            elif value_type == 2: #list
                table_id = int.from_bytes(value[2:], 'little')
                _value = SList(table_id)
            elif value_type == 3:#dict
                table_id = int.from_bytes(value[2:], 'little')
                _value = SDict(table_id)
            else:
                raise TypeError('unknown key type')

            self._list.insert(0,_value)

    def get_hash(self, v):
        if type(v) is int:
            return v
        elif type(v) in (str, bytes):
            return hash64(v)
        elif type(v) in (SDict, SList):
            return v.table_id
        else:
            raise TypeError('unsupported value type')

    def get_type(self,val):
        if type(val) == int:
            return 0
        elif type(val) == str:
            return 1
        elif type(val) == SList:
            return 2
        elif type(val) == SDict:
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
            raw_data = data.table_id
        elif data_type == 3: #dict
            raw_length = 8
            raw_data = data.table_id
        return (data_type, raw_length, raw_data)
    
    def __getitem__(self, index):
        return self._list[index]

    def __setitem__(self, key, val):
        if key > 0 and key < len(self._list) and val == self._list[key]:
            return

        id = key
        if self.object_type:
            _value = object_type.pack(val)
        else:
            value_type, raw_value_length, raw_value_data = self.get_raw_data(val)
            _value = ustruct.pack('B', value_type)
            _value += raw_value_data

        itr = db_find_i64(g_code, g_scope, self.table_id, id)
        if itr < 0:
            db_store_i64(g_scope, self.table_id, payer, id, _value)
        else:
            db_update_i64(itr, payer, _value)

    def push(self, value):
        self._list.append(value)
        return len(self._list)

    @property
    def length(self):
        return len(self._list)
    
    def __iter__(self):
        return iter(self._list)
    
    def __len__(self):
        return len(self._list)

    def __delitem__(self, key):
        id = key
        del self._list[key]
        itr = db_find_i64(g_code, g_scope, self.table_id, key)
        if itr >= 0:
            db_remove_i64(itr)

    def __repr__(self):
        return '%s(%s)' % (type(self).__name__, str(self._list))

# key_type key_length value_type value_length key_data value_data
class SDict(object):
    table_id_counter = 0
    def __init__(self, table_id = None, key_type = None, value_type = None):
        self._dict = {}
        if table_id:
            self.table_id = table_id
        else:
            SDict.table_id_counter += 1
            self.table_id = SDict.table_id_counter
            table_id = self.table_id

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
                _key = SList(table_id)
            elif key_type == 3:#dict
                table_id = int.from_bytes(value[8:8+key_length], 'little')
                _key = SDict(table_id)
            else:
                raise TypeError('unknown key type')

            if value_type == 0: #int
                _value = int.from_bytes(value[8+key_length:], 'little')
            elif value_type == 1: #str
                _value = value[8+key_length:]
            elif value_type == 2: #list
                table_id = int.from_bytes(value[8+key_length:], 'little')
                _value = SList(table_id)
            elif value_type == 3:#dict
                table_id = int.from_bytes(value[8+key_length:], 'little')
                _value = SDict(table_id)
            else:
                raise TypeError('unknown key type')

            self._dict[_key] = _value

    def get_hash(self, v):
        if type(v) is int:
            return v
        elif type(v) in (str, bytes):
            return hash64(v)
        elif type(v) in (SDict, SList):
            return v.table_id
        else:
            raise TypeError('unsupported value type')

    def get_type(self,val):
        if type(val) == int:
            return 0
        elif type(val) == str:
            return 1
        elif type(val) == SList:
            return 2
        elif type(val) == SDict:
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
        return '%s(%s)' % (type(self).__name__, str(self._dict))

def apply(name, type):
    require_auth(g_code)
    a = SDict(N('a'))
    b = SDict(N('b'))
    for key in a:
        print(key, a[key])
    a[100] = 'hello'
    a[101] = 'world'
    a['name'] = 'mike'
    b[0] = '0'
    b[1] = '1'
    a['b'] = b
    
    msg = read_action()
    a[msg] = msg
    b[msg] = msg
    if 101 in a:
        del a[101]

