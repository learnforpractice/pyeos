# cython: c_string_type=str, c_string_encoding=ascii
from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "net_.hpp":
    string connect_(const string& host)
    string disconnect_(const string& host)
    object status_(const string& host);
    object connections_()


def connect(string& host):
    return connect_(host)

def disconnect(string& host):
    return disconnect_(host)

def reconnect(string& host):
    disconnect_(host)
    connect_(host)

def status(string& host):
    return status_(host)

def connections():
    return connections_()
