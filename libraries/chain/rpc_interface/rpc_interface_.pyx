# cython: c_string_type=bytes, c_string_encoding=ascii
from libcpp.string cimport string
from threading import Thread
cdef extern from "rpc_interface.hpp":
    void init_eos() nogil
    void install_ctrl_c_handler()

cdef extern int rpc_interface_apply(unsigned long long account, unsigned long long action, string& code) with gil:
#    _account = int.to_bytes(account, 8, 'little')
    import eosserver
    ret = eosserver.apply(account, action, code)
    return ret

cdef extern void run_code_(string code):
    exec(code)

class Consolethread(Thread):
    def __init__(self):
        Thread.__init__(self)

    def run(self):
        init_eos();

def start_eos():
    t = Consolethread()
    t.start()

