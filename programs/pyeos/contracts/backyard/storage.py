import ustruct as struct
from eoslib import *

code = None
scope = None
payer = None

type_map = {int:0, str: 1, bytes: 2}

def pack_int(value):
    return b'\x00' + int.to_bytes(value, 8, 'little')

def unpack_int(value):
    return int.from_bytes(value[1:], 'little')

def pack_str(value):
    return b'\x01' + value.encode('utf8')

def unpack_str(value):
    return value[1:].decode('utf8')

def pack_bytes(value):
    return b'\x02' + value

def unpack_bytes(value):
    return value[1:]

pack_funcs = {0:pack_int, 1:pack_str, 2: pack_bytes}
unpack_funcs = {0:unpack_int, 1:unpack_str, 2:unpack_bytes}

def get_type_id(value):
    return type_map[type(value)]

def _pack(value):
    type_id = get_type_id(value)
    if type_id in pack_funcs:
        return pack_funcs[type_id](value)
    raise Exception('unknow type')

def _unpack(value):
    type_id = value[0]
    if type_id in unpack_funcs:
        return (type_id, unpack_funcs[type_id](value))
    return (value[0], value[1:])

def _get_hash(key):
    if type(key) is int:
        return key
    if type(key) is bytes and len(key) < 8:
        return int.from_bytes(key, 'little')
    if type(key) is str and len(key) < 8:
        return int.from_bytes(key, 'little')
    return hash64(key)

def storage_find(table_id, id):
    itr = db_find_i64(code, code, table_id, id)
    if itr >= 0:
        value = db_get_i64(itr)
        return _unpack(value)
    return None

def storage_get(itr):
    value = db_get_i64(itr)
    return _unpack(value)

def storage_set(table_id, key, value):
    itr = db_find_i64(code, code, table_id, key)
    if itr >= 0:
        db_update_i64(itr, code, value)
    else:
        db_store_i64(code, table_id, payer, key, value)

def storage_remove(table_id, key):
    itr = db_find_i64(code, code, table_id, key)
    if itr >= 0:
        db_remove_i64(itr)

class storage(object):
    def __init__(self):
        self.table_id = 0

class SList(storage):
    #FIXME
    table_counter = 0
    def __init__(self,table_id = None, value_type = None):
        self._dict = {}
        if table_id:
            self.table_id = table_id
        else:
            SList.table_counter += 1
            table_id = hash64('list.%d'%(SList.table_counter,))

#        print('current_receiver:', n2s(code))

        self.list_size_id = hash64('list.size%d')

        itr = db_find_i64(code, code, table_id, self.list_size_id)
        if itr >= 0:
            self.list_size = db_get_i64(itr)
            self.list_size = int.from_bytes(self.list_size, 'little')
        else:
            self.list_size = 0

        self.value_type = value_type
        
        itr = db_end_i64(code, scope, table_id)
        if itr == -1: #no value in table
            return
#        print('+++itr:', itr) # itr should be -2

    def get_type(self,val):
        if type(val) is int:
            return 0
        elif type(val) is str:
            return 1
        elif type(val) is bytes:
            return 2
        elif type(val) is SList:
            return 3
        elif type(val) is SDict:
            return 4
        else:
            raise TypeError('unsupported type')
    
    def pack(self, data):
        if self.value_type:
            return self.value_type.pack(data)
        data_type = self.get_type(data)
        raw_data = 0
        raw_length = 0
        if data_type == 0: #int
            raw_length = 8
            raw_data = int.to_bytes(data, 8, 'little')
        elif data_type == 1: #str
            raw_length = len(data)
            raw_data = data.encode('utf8')
        elif data_type == 2: #bytes
            raw_length = len(data)
            raw_data = data
        elif data_type == 3: #SList
            raw_length = 8
            raw_data = data.table_id
        elif data_type == 4: #SDict
            raw_length = 8
            raw_data = data.table_id

        _value = int.to_bytes(data_type, 1, 'little')
        _value += raw_data
        return _value

    def unpack(self, data):
        if self.value_type:
            return value_type.unpack(data)
        value_type = data[0]
        if value_type == 0: #int
            _value = int.from_bytes(data[1:], 'little')
        elif value_type == 1: #str
            _value = data[1:].decode('utf8')
        elif value_type == 2: #bytes
            _value = data[1:].decode('utf8')
        elif value_type == 3: #SList
            table_id = int.from_bytes(data[1:], 'little')
            _value = SList(table_id)
        elif value_type == 4:#SDict
            table_id = int.from_bytes(data[1:], 'little')
            _value = SDict(table_id)
        else:
            raise TypeError('unknown key type')
        return _value
    
    def get_hash(self, v):
        if type(v) is int:
            return v
        elif type(v) in (str, bytes):
            return hash64(v)
        elif type(v) in (SDict, SList):
            return v.table_id
        else:
            raise TypeError('unsupported value type')

    def update_size(self):
        _value = int.to_bytes(self.list_size, 4, 'little')
        itr = db_find_i64(code, scope, self.table_id, self.list_size_id)
        if itr < 0:
            db_store_i64(scope, self.table_id, payer, self.list_size_id, _value)
        else:
            db_update_i64(itr, payer, _value)

    def setitem(self, index, val):
        id = index
        if self.value_type:
            _value = val.pack()
        else:
            _value = self.pack(val)

        itr = db_find_i64(code, scope, self.table_id, id)
        if itr < 0:
            db_store_i64(scope, self.table_id, payer, id, _value)
        else:
            db_update_i64(itr, payer, _value)

    def __setitem__(self, index, val):
        if index < 0 or index >= self.list_size:
            raise IndexError('list assignment index out of range')
        if index in self._dict and self._dict[index] == val:
            return

        self.setitem(index, val)

    def __getitem__(self, index):
        if index < 0 or index >= self.list_size:
            raise IndexError('list assignment index out of range')
        if index in self._dict:
            return self._dict[index]

        itr = db_find_i64(code, scope, self.table_id, index)
        if itr >= 0:
            value = db_get_i64(itr)
            value = self.unpack(value)
            self._dict[index] = value
            return value
        raise Exception('unkown error!') # code should never go here!

    def insert(self, value):
        raise Exception('insert is not supported in SList!')

    def append(self, value):
        self.list_size += 1
        self.setitem(self.list_size-1, value)
        self.update_size()

    def push(self, value):
        self.list_size += 1
        self.setitem(self.list_size-1, value)
        self.update_size()
        return self.list_size

    @property
    def length(self):
        return self.list_size

    def __iter__(self):
        #FIXME
        raise Exception("iterator is not supported in SList!")
#        return iter(self._list)

    def __len__(self):
        return self.list_size

    def __delitem__(self, key):
        raise Exception("delete is not supported in SList!")

    def __repr__(self):
        #FIXME
        raise Exception("__repr__ is not supported in SList!")
#        return '%s(%s)' % (type(self).__name__, str(self._dict))

# key_type key_length value_type value_length key_data value_data
class SDict(storage):
    table_counter = 0
    def __init__(self, table_id = None, key_type = None, value_type = None):
        self._dict = {}
        if table_id:
            self.key_table_id = hash64('dict.key.%d'%(table_id,))
            self.value_table_id = hash64('dict.value.%d'%(table_id,))
        else:
            SDict.table_counter += 1
            self.key_table_id = hash64('dict.key.%d'%(SDict.table_counter,))
            self.value_table_id = hash64('dict.value.%d'%(SDict.table_counter,))
        self.table_id = table_id
        
        self.key_type = key_type
        self.value_type = value_type

    def pack(self, obj):
        data = None
        if type(obj) is SList:
            data = b'\x03'
            data += int.to_bytes(obj.table_id, 8, 'little')
        elif type(obj) is SDict:
            data = b'\x04'
            data += int.to_bytes(obj.table_id, 8, 'little')
        else:
            data = _pack(obj)
        return data

    def unpack(self, type_id, data):
        if type_id == 3:
            table_id = int.from_bytes(data, 'little')
            return SList(table_id=table_id)
        elif type_id == 4:
            table_id = int.from_bytes(data, 'little')
            return SDict(table_id=table_id)
        return data

    def get_hash(self, v):
        if isinstance(v, storage):
            return v.table_id
        else:
            return _get_hash(v)

    def getitem(self, key):
        if key in self._dict:
            return self._dict[key]
        
        id = self.get_hash(key)
        
        res = storage_find(self.key_table_id, key)
        if not res:
            raise KeyError

        type_id, key = res
        key = self.unpack(type_id, key)
 
        type_id, value = storage_find(self.key_table_id, key)
        value = self.unpack(type_id, value)

        self._dict[key] = value
        return value

    def find(self, key):
        if key in self._dict:
            return self._dict[key]
        
        id = self.get_hash(key)
        
        res = storage_find(self.key_table_id, key)
        if not res:
            return False

        type_id, key = res
        key = self.unpack(type_id, key)
 
        type_id, value = storage_find(self.key_table_id, key)
        value = self.unpack(type_id, value)

        self._dict[key] = value
        return value

    def __getitem__(self, key):
        return self.getitem(key)

    def __setitem__(self, key, val):
        if key in self._dict and val == self._dict[key]:
            return

        self._dict[key] = val
        id = self.get_hash(key)

        raw_key = self.pack(key)
        if self.value_type:
            raw_val = val.pack()
        else:
            raw_val = self.pack(val)

        storage_set(self.key_table_id, id, raw_key)
        storage_set(self.value_table_id, id, raw_val)

    def __iter__(self):
        self.idx = db_end_i64(code, scope, self.key_table_id)
        return self

    def __next__(self):
        if self.idx == -1:
            raise StopIteration
        self.idx, id = db_previous_i64(self.idx)
        if self.idx == -1:
            raise StopIteration
        type_id, key = storage_get(self.idx)
#        print(type_id, key)
        if not key:
            raise StopIteration 
        key = self.unpack(type_id, key)
        '''
        type_id, value = storage_find(self.value_table_id, key)
        if not value:
            raise StopIteration 
        value = self.unpack(value)
        self._dict[key] = value
        '''
        return key

    def __len__(self):
        return len(self._dict)

    def __delitem__(self, key):
        id = self.get_hash(key)
        storage_remove(self.key_table_id, id)
        storage_remove(self.value_table_id, id)
        if key in self._dict:
            del self._dict[key]

    def __repr__(self):
        return '%s(%s)' % (type(self).__name__, str(self.table_id))
        raise Exception("repr is not supported in SDict")

def apply(name, type):
    global code
    global scope
    global payer
    code = current_receiver()
#    print('++++current_receiver:', n2s(code))
    scope = code
    payer = code

    if type == N('slisttest'):
        sl = SList(table_id=1)
#        print('+++++++++len(sl):', len(sl))
        for i in range(10): #len(sl)):
            sl[i]
#            print(sl[i])
        sl.append(read_action())

    elif type == N('sdicttest'):
        a = SDict(N('a'))
        b = SDict(N('b'))
#        for key in a:
#            print('==========', key) #, a[key])
        a[100] = 'hello'
        a[101] = 'world'
        a['name'] = 'mike'
        b[0] = '0'
        b[1] = '1'
        a['b'] = b

        msg = read_action()
        msg = msg.decode('utf8')
        a[msg] = msg
        b[msg] = msg
#        if 101 in a:
#            del a[101]

