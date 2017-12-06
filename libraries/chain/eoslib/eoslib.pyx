from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "eoslib_.hpp":
    ctypedef unsigned long long uint64_t
    ctypedef int int32_t
    ctypedef unsigned int uint32_t
    cdef cppclass Name:
        Name()
        Name(uint64_t)
    uint64_t string_to_uint64_(string str);
    string uint64_to_string_(uint64_t n);
    
    void requireAuth_(uint64_t account);
    int readMessage_(string& buffer);
    void requireScope_(uint64_t account);
    void requireNotice_(uint64_t account);
    uint64_t currentCode_();
    uint32_t now_();

    int32_t store_(Name scope, Name table, void* keys, int keyslen, int key_type, char* value, uint32_t valuelen);
    int32_t update_(Name scope, Name table, void* keys, int keyslen, int key_type, char* value, uint32_t valuelen);
    int32_t remove_(Name scope, Name table, void* keys, int keyslen, int key_type, char* value, uint32_t valuelen);
    int32_t load_(Name scope, Name code, Name table, void* keys, int keyslen, int key_type, int scope_index, char* value, uint32_t valuelen);
    int32_t front_(Name scope, Name code, Name table, void* keys, int keyslen, int key_type, int scope_index, char* value, uint32_t valuelen)
    int32_t back_(Name scope, Name code, Name table, void* keys, int keyslen, int key_type, int scope_index, char* value, uint32_t valuelen)
    int32_t next_(Name scope, Name code, Name table, void* keys, int keyslen, int key_type, int scope_index, char* value, uint32_t valuelen)
    int32_t previous_(Name scope, Name code, Name table, void* keys, int keyslen, int key_type, int scope_index, char* value, uint32_t valuelen)
    int32_t lower_bound_(Name scope, Name code, Name table, void* keys, int keyslen, int key_type, int scope_index, char* value, uint32_t valuelen)
    int32_t upper_bound_(Name scope, Name code, Name table, void* keys, int keyslen, int key_type, int scope_index, char* value, uint32_t valuelen)

    void unpack_(string& raw, string& out)
    void pack_(string& raw, string& out)

cdef extern from "<eos/chain/python_interface.hpp>" namespace "eosio::chain":
    int wasm_call_function(uint64_t code, uint64_t function, vector[uint64_t] args);
    uint32_t transactionCreate_();
    void transactionRequireScope_(uint32_t handle, string& scope, uint32_t readOnly);
    void transactionAddMessage_(uint32_t handle,uint32_t msg_handle);
    void transactionSend_(uint32_t handle);
    void transactionDrop_(uint32_t handle);
    uint32_t messageCreate_(string& code, string& type, string& data);
    void messageRequirePermission_(uint32_t handle, string& account,string& permission);
    void messageSend_(uint32_t handle);
    void messageDrop_(uint32_t handle);


KEY_TYPE_KEY = 0
KEY_TYPE_KEY128x128 = 1
KEY_TYPE_KEY64x64x64 = 2
KEY_TYPE_STR = 3

def pack(bytes bs):
    cdef string out
    pack_(bs, out)
    return out

def unpack(bytes bs):
    cdef string out
    unpack_(bs, out)
    return out

def now():
    return now_()

cdef uint64_t toname(name):
    if isinstance(name, int):
        return name
    if isinstance(name, str):
        name = bytes(name, 'utf8')
    return string_to_uint64_(name)

def s2n(name):
    return toname(name)

def N(name):
    return s2n(name)

def n2s(uint64_t n):
    return uint64_to_string_(n)

def requireAuth(name):
    cdef uint64_t name_
    name_ = toname(name)
    requireAuth_(name_)

def readMessage():
    cdef string buffer
    cdef int ret
    ret = readMessage_(buffer)
    return buffer

def requireScope(account):
    cdef uint64_t account_
    account_ = toname(account)
    requireScope_(account_)

def requireNotice(account):
    cdef uint64_t account_
    account_ = toname(account)
    requireNotice_(account_)

def currentCode():
    return currentCode_()

def store(scope, table, bytes keys, int key_type, bytes value):
    cdef uint64_t scope_
    cdef uint64_t table_
    cdef char* keys_ = keys
    scope_ = toname(scope)
    table_ = toname(table)
    return store_(Name(scope_), Name(table_), <void*>keys_, len(keys), key_type, value, len(value))

def store_u64(scope, table, uint64_t key, uint64_t value):
    cdef uint64_t scope_
    cdef uint64_t table_
    scope_ = toname(scope)
    table_ = toname(table)
    return store_(Name(scope_), Name(table_), <void*>&key, sizeof(key), 0, < char *>&value, sizeof(value))

def update(scope, table, bytes keys, int key_type, bytes value):
    cdef uint64_t scope_
    cdef uint64_t table_
    cdef char* keys_ = keys
    scope_ = toname(scope)
    table_ = toname(table)
    return update_(Name(scope_), Name(table_), <void*>keys_, len(keys), key_type, value, len(value))

def remove(scope, table, bytes keys, int key_type, bytes value):
    cdef uint64_t scope_
    cdef uint64_t table_
    cdef char* keys_ = keys
    scope_ = toname(scope)
    table_ = toname(table)
    return remove_(Name(scope_), Name(table_), <void*>keys_, len(keys), key_type, value, len(value))

def load(scope, code, table, bytes keys, int key_type, int scope_index, bytes values):
    cdef uint64_t scope_
    cdef uint64_t code_
    cdef uint64_t table_
    cdef char* keys_ = keys
    cdef char* values_ = values
    
    scope_ = toname(scope)
    code_ = toname(code)
    table_ = toname(table)
    return load_(Name(scope_), Name(code_), Name(table_), <void*>keys_, len(keys), key_type, scope_index, values_, len(values))

def load_u64(scope, code, table, uint64_t key):
    cdef uint64_t scope_
    cdef uint64_t code_
    cdef uint64_t table_
    cdef uint64_t value
    cdef int value_length

    scope_ = toname(scope)
    code_ = toname(code)
    table_ = toname(table)
    value = 0
    value_length = load_(Name(scope_), Name(code_), Name(table_), <void*>&key, sizeof(key), 0, 0, < char *>&value, sizeof(value))
    return value

'''
int32_t front_( Name scope, Name code, Name table, void* keys,int key_type,int scope_index, char* value, uint32_t valuelen )
int32_t back_( Name scope, Name code, Name table, void* keys,int key_type, int scope_index, char* value, uint32_t valuelen )
int32_t next_( Name scope, Name code, Name table, void* keys,int key_type, int scope_index, char* value, uint32_t valuelen )
int32_t previous_( Name scope, Name code, Name table, void* keys,int key_type, int scope_index, char* value, uint32_t valuelen )
int32_t lower_bound_( Name scope, Name code, Name table, void* keys,int key_type, int scope_index, char* value, uint32_t valuelen )
int32_t upper_bound_( Name scope, Name code, Name table, void* keys,int key_type, int scope_index, char* value, uint32_t valuelen )
'''
cdef int get_key_size(int key_type):
    if key_type == 0:
        return 64 / 8
    elif key_type == 1:
        return 128 * 2 / 8
    elif key_type == 2:
        return 64 * 3 / 8
    return 0

def front(scope, code, table, bytes keys, int key_type, int scope_index, bytes values):
    cdef uint64_t scope_
    cdef uint64_t code_
    cdef uint64_t table_
    cdef char* keys_ = keys
    cdef char* values_ = values
    cdef int value_length
    scope_ = toname(scope)
    code_ = toname(code)
    table_ = toname(table)
    if get_key_size(key_type) != len(keys):
        return None
    if key_type > 2:
        return None
    return front_(Name(scope_), Name(code_), Name(table_), <void*>keys_, len(keys), key_type, scope_index, values_, len(values))

def back(scope, code, table, bytes keys, int key_type, int scope_index, bytes values):
    cdef uint64_t scope_
    cdef uint64_t code_
    cdef uint64_t table_
    cdef char* keys_ = keys
    cdef char* values_ = values
    scope_ = toname(scope)
    code_ = toname(code)
    table_ = toname(table)
    if get_key_size(key_type) != len(keys):
        return None
    if key_type > 2:
        return None
    return back_(Name(scope_), Name(code_), Name(table_), <void*>keys_, len(keys), key_type, scope_index, values_, len(values))

def next(scope, code, table, bytes keys, int key_type, int scope_index, bytes values):
    cdef uint64_t scope_
    cdef uint64_t code_
    cdef uint64_t table_
    cdef char* keys_ = keys
    cdef char* values_ = values

    scope_ = toname(scope)
    code_ = toname(code)
    table_ = toname(table)
    return next_(Name(scope_), Name(code_), Name(table_), <void*>keys_, len(keys), key_type, scope_index, values_, len(values))

def previous(scope, code, table, bytes keys, int key_type, int scope_index, bytes values):
    cdef uint64_t scope_
    cdef uint64_t code_
    cdef uint64_t table_
    cdef char* keys_ = keys
    cdef char* values_ = values

    scope_ = toname(scope)
    code_ = toname(code)
    table_ = toname(table)
    return previous_(Name(scope_), Name(code_), Name(table_), <void*>keys_, len(keys), key_type, scope_index, values_, len(values))

def lower_bound(scope, code, table, bytes keys, int key_type, int scope_index, bytes values):
    cdef uint64_t scope_
    cdef uint64_t code_
    cdef uint64_t table_
    cdef char* keys_ = keys
    cdef char* values_ = values

    scope_ = toname(scope)
    code_ = toname(code)
    table_ = toname(table)
    return lower_bound_(Name(scope_), Name(code_), Name(table_), <void*>keys_, len(keys), key_type, scope_index, values_, len(values))

def upper_bound(scope, code, table, bytes keys, int key_type, int scope_index, bytes values):
    cdef uint64_t scope_
    cdef uint64_t code_
    cdef uint64_t table_
    cdef char* keys_ = keys
    cdef char* values_ = values

    scope_ = toname(scope)
    code_ = toname(code)
    table_ = toname(table)
    return upper_bound_(Name(scope_), Name(code_), Name(table_), <void*>keys_, len(keys), key_type, scope_index, values_, len(values))


#str
def store_str(scope, table, bytes keys, bytes value):
    return store(scope, table, keys, KEY_TYPE_STR, value)

def update_str(scope, table, bytes keys, bytes value):
    return update(scope, table, keys, KEY_TYPE_STR, value)

def remove_str(scope, table, bytes keys, bytes value):
    return remove(scope, table, keys, KEY_TYPE_STR, value)

def load_str(scope, code, table, bytes keys):
    values = bytes(256)
    ret = load(scope, code, table, keys, KEY_TYPE_STR, 0, values)
    if ret > 0:
        return values[:ret]
    return None

def front_str(scope, code, table, bytes keys):
    values = bytes(256)
    ret = front(scope, code, table, keys, KEY_TYPE_STR, 0, values)
    if ret > 0:
        return values[:ret]
    return None

def back_str(scope, code, table, bytes keys):
    values = bytes(256)
    ret = back(scope, code, table, keys, KEY_TYPE_STR, 0, values)
    if ret > 0:
        return values[:ret]
    return None

def next_str(scope, code, table, bytes keys):
    values = bytes(256)
    ret = next(scope, code, table, keys, KEY_TYPE_STR, 0, values)
    if ret > 0:
        return values[:ret]
    return None

def previous_str(scope, code, table, bytes keys):
    values = bytes(256)
    ret = previous(scope, code, table, keys, KEY_TYPE_STR, 0, values)
    if ret > 0:
        return values[:ret]
    return None

def lower_bound_str(scope, code, table, bytes keys):
    values = bytes(256)
    ret = lower_bound(scope, code, table, keys, KEY_TYPE_STR, 0, values)
    if ret > 0:
        return values[:ret]
    return None

def upper_bound_str(scope, code, table, bytes keys):
    values = bytes(256)
    ret = upper_bound(scope, code, table, keys, KEY_TYPE_STR, 0, values)
    if ret > 0:
        return values[:ret]
    return None


def call_wasm_function(uint64_t code, uint64_t function, args: list):
    cdef vector[uint64_t] args_
    for arg in args:
        args_.push_back(arg)
    return wasm_call_function(code, function, args_);


def transactionCreate():
    return transactionCreate_();

def transactionRequireScope(uint32_t handle, string scope, readOnly=0):
    if readOnly:
        readOnly = 1
    else:
        readOnly = 0
    transactionRequireScope_(handle, scope, readOnly);

def transactionAddMessage(uint32_t handle,uint32_t msg_handle):
    transactionAddMessage_(handle,msg_handle);

def transactionSend(uint32_t handle):
    transactionSend_(handle);

def transactionDrop(uint32_t handle):
    transactionDrop_(handle);

def messageCreate(string code, string type, string data):
    return messageCreate_(code, type, data);

def messageRequirePermission(uint32_t handle, string& account,string& permission):
    messageRequirePermission_(handle, account, permission);

def messageSend(uint32_t handle):
    messageSend_(handle);

def messageDrop_(uint32_t handle):
    messageDrop_(handle);


