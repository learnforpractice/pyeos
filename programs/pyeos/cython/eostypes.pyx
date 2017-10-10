import struct
from libcpp.string cimport string

MAX_INT64 = 0xffffffffffffffff
precision = int.to_bytes(1000*1000*1000*1000*1000,16,'little')

cdef extern from "math_.hpp":
    void mul64_(char *n1,char *n2,char *out);
    void div64_(char *n1,char *n2,char *out);
    void add64_(char *n1,char *n2,char *out);
    void sub64_(char *n1,char *n2,char *out);
    void mul128_(char *n1,char *n2,char *out);
    void div128_(char *n1,char *n2,char *out);
    void add128_(char *n1,char *n2,char *out);
    void sub128_(char *n1,char *n2,char *out);

def mul64(bytes n1,bytes n2,bytes out):
    assert len(n1) == 8
    assert len(n2) == 8
    assert len(out) == 8
    mul64_(n1,n2,out)

def div64(bytes n1,bytes n2,bytes out):
    assert len(n1) == 8
    assert len(n2) == 8
    assert len(out) == 8
    div64_(n1,n2,out)

def add64(bytes n1,bytes n2,bytes out):
    assert len(n1) == 8
    assert len(n2) == 8
    assert len(out) == 8
    add64_(n1,n2,out)
    
def sub64(bytes n1,bytes n2,bytes out):
    assert len(n1) == 8
    assert len(n2) == 8
    assert len(out) == 8
    sub64_(n1,n2,out)    

def mul128(bytes n1,bytes n2,bytes out):
    assert len(n1) == 16
    assert len(n2) == 16
    assert len(out) == 16
    mul128_(n1,n2,out)

def div128(bytes n1,bytes n2,bytes out):
    assert len(n1) == 16
    assert len(n2) == 16
    assert len(out) == 16
    div128_(n1,n2,out)

def add128(bytes n1,bytes n2,bytes out):
    assert len(n1) == 16
    assert len(n2) == 16
    assert len(out) == 16
    add128_(n1,n2,out)
    
def sub128(bytes n1,bytes n2,bytes out):
    assert len(n1) == 16
    assert len(n2) == 16
    assert len(out) == 16
    sub128_(n1,n2,out)

class uint64(int):
    def __init__(self,n=0):
        if isinstance(n,bytes):
            assert len(n) == 8
            self.n = n
        else:
            self.n = int.to_bytes(n,8,'little')
    def from_bytes(bs):
        assert len(bs) == 8
        n = uint64()
        n.n = bytes(bs)
        return n
    def __call__(self):
#        return struct.pack('QQ',self & MAX_INT64,self >> 64)
        return self.to_bytes(8,'little')
    def __div__(self,other):
        out = bytes(8)
        div64(self.n,other.n,out)
        return uint64(out)
    def __truediv__(self,other):
        out = bytes(8)
        div64(self.n,other.n,out)
        return uint64(out)
    def __mul__(self,other):
        out = bytes(8)
        mul64(self.n,other.n,out)
        return uint64(out)
    def __add__(self,other):
        out = bytes(8)
        add64(self.n,other.n,out)
        return uint64(out)
    def __sub__(self,other):
        out = bytes(8)
        sub64(self.n,other.n,out)
        return uint64(out)
    def __str__(self):
        return str(int.from_bytes(self.n,'little'))
    def __repr__(self):
        return str(self.n)

class uint128(object):
    def __init__(self,n=0):
        if isinstance(n,bytes):
            assert len(n) == 16
            self.n = n
        else:
            self.n = int.to_bytes(n,16,'little')
    def from_bytes(bs):
        assert len(bs) == 16
        n = uint128()
        n.n = bytes(bs)
        return n
    def __call__(self):
#        return struct.pack('QQ',self & MAX_INT64,self >> 64)
        return self.to_bytes(16,'little')
    def __div__(self,other):
        out = bytes(16)
        div128(self.n,other.n,out)
        return uint128(out)
    def __truediv__(self,other):
        out = bytes(16)
        div128(self.n,other.n,out)
        return uint128(out)
    def __mul__(self,other):
        out = bytes(16)
        mul128(self.n,other.n,out)
        return uint128(out)
    def __add__(self,other):
        out = bytes(16)
        add128(self.n,other.n,out)
        return uint128(out)
    def __sub__(self,other):
        out = bytes(16)
        sub128(self.n,other.n,out)
        return uint128(out)
    def __str__(self):
        return str(int.from_bytes(self.n,'little'))
    def __repr__(self):
        return str(self.n)
