cdef extern from "<eosio/chain/micropython_interface.hpp>":
    void* execute_from_str(const char *str);

def micropy_eval(code):
    if isinstance(code, str):
        code = code.encode('utf8')
    execute_from_str(code)


