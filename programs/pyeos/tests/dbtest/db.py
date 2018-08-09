import db
import struct
from eoslib import *

type_map = {int:'Q', bytes:'s'}


class SList(object):
    def __init__(self, code, scope, table_id):
        self.code = code
        self.scope = scope
        self.table_id = table_id
        self.count = db.get_table_item_count(code, scope, table_id)
        print('table count:', self.count)

    def pack(self, obj):
        if type(obj) is int:
            return struct.pack('QB', obj, 0)
        elif type(obj) is bytes:
            return struct.pack('%dsB'%(len(obj),), obj, 1)
        raise Exception('unsupported type', type(obj))
    
    def unpack(self, bs):
        if bs[-1] == 0:
            return struct.unpack('Q', bs[:8])[0]
        elif bs[-1] == 1:
            return bs[:-1]

    def __setitem__(self, index, val):
        if index < 0 or index >= self.count:
            raise IndexError('list index out of range')
        db.update_i64(index, self.code, self.pack(val))

    def __getitem__(self, index):
        if index < 0 or index >= self.count:
            raise IndexError('list index out of range')
        print('index', index)
        itr = db.find_i64(self.code, self.scope, self.table_id, index)
        if itr < 0:
            raise Exception('index not found!')
        print('index', index, 'it', itr)
        value = db.get_i64(itr)
        return self.unpack(value)

    def append(self, value):
        print('+++append:', value)
        db.store_i64(self.scope, self.table_id, self.code, self.count, self.pack(value))
        self.count += 1
        print('+++append return:', self.count)

    def __len__(self):
        return self.count

class SDict(object):
    def __init__(self, code, scope, table_id):
        self.code = code
        self.scope = scope
        self.table_id = table_id
        self.count = db.get_table_item_count(code, scope, table_id)
        print('table count:', self.count)

    def pack(self, obj):
        if type(obj) is int:
            return struct.pack('QB', obj, 0)
        elif type(obj) is bytes:
            return struct.pack('%dsB'%(len(obj),), obj, 1)
        raise Exception('unsupported type', type(obj))
    
    def unpack(self, bs):
        if bs[-1] == 0:
            return struct.unpack('Q', bs[:8])[0]
        elif bs[-1] == 1:
            return bs[:-1]

    def __setitem__(self, index, val):
        db.update_i64(index, self.code, self.pack(val))

    def __getitem__(self, index):
        if index < 0 or index >= self.count:
            raise IndexError('list index out of range')
        print('index', index)
        itr = db.find_i64(self.code, self.scope, self.table_id, index)
        if itr < 0:
            raise Exception('index not found!')
        print('index', index, 'it', itr)
        value = db.get_i64(itr)
        return self.unpack(value)

    def append(self, value):
        print('+++append:', value)
        db.store_i64(self.scope, self.table_id, self.code, self.count, self.pack(value))
        self.count += 1
        print('+++append return:', self.count)

    def remove(self, index):
        print('+++append:', value)
        itr = db.find_i64(self.code, self.scope, self.table_id, index)
        if itr <= 0:
            raise Exception('index not found', index)
        db.remove_i64(itr)
        self.count -= 1
        print('+++append return:', self.count)

    def __len__(self):
        return self.count


def apply(receiver, code, action):
    if action == N('sayhello'):
        msg = read_action()
        sl = SList(code, code, N('mytable'))
        try:
            print(sl[0])
        except Exception as e:
            print(e)
        sl.append(msg)
        sl[0] = msg
