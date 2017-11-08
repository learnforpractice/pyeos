from libcpp.string cimport string
from libcpp.vector cimport vector
import imp
import logging as log
import tracemalloc
import traceback
import eoslib


cdef extern from "<fc/log/logger.hpp>":
    void ilog(string& str)
    ctypedef unsigned long long uint64_t

code_map = {}

cdef extern int python_load(string& name, string& code, string* error):
    global code_map
    cdef int ret
    cdef string str_error
    
    py_error = None
    ret = 0
#    print('python_load:',name)
    module = code_map.get(name)
    cdef bytes code_ = code
    if not module or (module.__code != code_):
        try:
            new_module = imp.new_module(str(name))
            tracemalloc.stop()
            tracemalloc.start()
            exec(code,vars(new_module))
            code_map[name] = new_module
            new_module.__code = code
            tracemalloc.stop()
        except Exception as e:
            tracemalloc.stop()
            py_error = traceback.format_exc()
#            log.exception(e)
            ret = -1

    if py_error:
        error[0] = bytes(py_error,'utf8')

    return ret;

cdef extern int python_call(string& name, string& function, vector[uint64_t] args, string* error):
    global code_map
    cdef int ret
    ret = -1
    py_error = None
    func = function
    func = func.decode('utf8')
#    print('python_call:',func)
    try:
        module = code_map[name]
        func = getattr(module,func)
        tracemalloc.stop()
        tracemalloc.start()
        func(*args)
        tracemalloc.stop()
        ret = 0
    except Exception as e:
        tracemalloc.stop()
        py_error = traceback.format_exc()
        ret = -1
#        log.exception(e)
    if py_error:
        error[0] = bytes(py_error,'utf8')
    return ret
