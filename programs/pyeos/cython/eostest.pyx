from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "eostest_.hpp":
    void test_all_();
    void test_db_();

def test_all():
    test_all_()

def test_db():
    test_db_();


