# cython: c_string_type=bytes, c_string_encoding=ascii

from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "ipc_.hpp":
    ctypedef unsigned long long uint64_t
    void* ipc_create(string& name, int max_num_msg, int max_msg_size) nogil
    void* ipc_open(string& name) nogil
    int ipc_receive(void* _mq, vector[char] buffer) nogil
    int ipc_send(void* _mq, string& _buffer) nogil
    void ipc_remove(string& name) nogil

def open(string& name):
    return <uint64_t>ipc_open(name);

def create(string& name, int max_num_msg, int max_msg_size):
   return <uint64_t>ipc_create(name, max_num_msg, max_msg_size)

def receive(uint64_t mq, size_t buffer_size):
    cdef size_t received_size;
    cdef string ret
    cdef vector[char] buffer
    buffer.resize(buffer_size)
    with nogil:
        received_size = ipc_receive(<void*>mq, buffer);
    ret = string(buffer.data(), received_size)
    return <bytes>ret

def send(uint64_t mq, string& _buffer):
    cdef int ret
    with nogil:
        ret = ipc_send(<void*>mq, _buffer);
    return ret

def remove(string& name):
    ipc_remove(name)

