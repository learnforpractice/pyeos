from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "eostest_.hpp" namespace "python::test":
    void test_all_();
    void test_db_();
    void test_create_account_(string& name)
    void test_get_account_(string& name)

    void test_start_();
    void test_create_account_();
    void produce_block_();
    void test_end_();

    void create_account_manually_(string& name);
    void get_account_(string& name);
    void get_block_num_();
    void get_block_log_num_();


def test_all():
    test_all_()

def test_db():
    test_db_();

def test_create_account(string name):
    test_create_account_(name)

def test_get_account(string name):
    test_get_account_(name)

def start():
    test_start_();

def create_account():
    test_create_account_();

def create_account_manually(string name):
    create_account_manually_(name);

def get_account(string name):
    get_account_(name);


def produce_block():
    produce_block_()

def get_block_num():
    get_block_num_()

def get_block_log_num():
    get_block_log_num_()


def end():
    test_end_();


