import struct
from libcpp.vector cimport vector
from libcpp.string cimport string
include "eostypes_.pxd"

cdef extern from "common.hpp":
    void emplace_scope(const name& scope, vector[name]& scopes)
    void vector_to_string(vector[char]& v, string& str);
    void string_to_vector(string& str, vector[char]& v);

cdef class PyMessage:
    cdef message* _thisptr

    def __cinit__(self, uint64_t ptr = 0):
        self._thisptr = new message()
        if ptr:
            self._thisptr[0] = (<message*><void *>ptr)[0]

    def init(self, code: bytes, type: bytes, author: list, data: bytes):
        self._thisptr.code = name(code)
        self._thisptr.type = name(type)

        for a in author:
            self._thisptr.authorization.push_back(account_permission(name(a[0]), name(a[1])))

        if data:
            string_to_vector(data, self._thisptr.data)

    def __dealloc__(self):
#        print('__dealloc__ PyMessage', self._borrowptr, <uint64_t>self._thisptr)
        if self._thisptr:
            del self._thisptr
            self._thisptr = NULL

    def __call__(self):
        return <uint64_t>self._thisptr

    def require_permission(self, account: bytes, permission: bytes):
        self._thisptr.authorization.push_back(account_permission(name(account), name(permission)))

    @property
    def code(self):
        return self._thisptr.code.to_string()

    @property
    def type(self):
        return self._thisptr.type.to_string()

    @property
    def data(self):
        cdef string str
#        print("self._thisptr.data.size():", self._thisptr.data.size())
        vector_to_string(self._thisptr.data, str)
        return str

cdef class PyTransaction:
    cdef transaction* _thisptr

    def __cinit__(self, uint64_t ptr = 0):
        self._thisptr = new transaction()
        if ptr:
            self._thisptr[0] = (<transaction*><void *>ptr)[0]

    def __dealloc__(self):
#        print('__dealloc__ PyTransaction', self._borrowptr, <uint64_t>self._thisptr)
        if self._thisptr:
            del self._thisptr
            self._thisptr = NULL

    def __call__(self):
        return <uint64_t>self._thisptr

    def init(self, refBlocNum = 0, refBlockPrefix = 0, expiration = 0, scopes = None, messages = None):
        self._thisptr.ref_block_num = refBlocNum
        self._thisptr.ref_block_prefix = refBlockPrefix
        self._thisptr.expiration = time_point_sec(expiration)
        
        if scopes:
            for scope in scopes:
                emplace_scope(name(scope),self._thisptr.scope)

        if messages:
            for msg in messages:
                self.add_message(msg)

    def reqire_scope(self, scope, read_only = False):
        if read_only:
            emplace_scope(name(scope), self._thisptr.read_scope)
        else:
            emplace_scope(name(scope), self._thisptr.scope)

    def add_message(self, msg):
        cdef message *msg_ptr
        cdef uint64_t ptr = msg()
        msg_ptr = <message*>ptr
        if msg:
            self._thisptr.messages.push_back(msg_ptr[0])

#    @property
    def get_messages(self):
        msgs = []
        for i in range(self._thisptr.messages.size()):
            msg = PyMessage(<uint64_t>&self._thisptr.messages[i])
            msgs.append(msg)
        '''bad code, local native_msg
        for native_msg in self._thisptr.messages:
            msg = PyMessage(<uint64_t>&native_msg)
            print('---------++++----------:',msg.code)
            msgs.append(msg)
        '''
        return msgs

cdef class PySignedTransaction(PyTransaction):

    def __cinit__(self,uint64_t ptr = 0):
        self._thisptr = <signed_transaction*>new signed_transaction()
        if ptr:
            self._thisptr[0] = (<signed_transaction*><void *>ptr)[0]

    def __dealloc__(self):
#        print('__dealloc__ PySignedTransaction', self._borrowptr, <uint64_t>self._thisptr)
        if self._thisptr:
            temp = <signed_transaction*>self._thisptr
            del temp
            self._thisptr = NULL

'''
    struct signed_transaction : public transaction { 
        signed_transaction() = default;
        signed_transaction(const Vector<Signature>& signatures)
           : signatures(signatures) {}

        Vector<Signature>                signatures;
    };
'''

