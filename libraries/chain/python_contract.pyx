from libcpp.string cimport string
from libcpp.vector cimport vector
import imp
import logging as log

cdef extern from "<fc/log/logger.hpp>":
    void ilog(string& str)
    ctypedef unsigned long long uint64_t

code_map = {}

cdef extern int python_load(string& name,string& code):
    global code_map
    cdef int ret
    ret = 0
    print('python_load:',name)
    module = code_map.get(name)
    cdef bytes code_ = code
    if not module or (module.__code != code_):
        try:
            new_module = imp.new_module(str(name))
            exec(code,vars(new_module))
            code_map[name] = new_module
            new_module.__code = code
        except Exception as e:
            log.exception(e)
            ret = -1
    return ret;

cdef extern python_call(string& name,string& function,vector[uint64_t] args):
    global code_map
    cdef int ret
    ret = -1
    func = function
    func = func.decode('utf8')
    print('python_call:',func)
    try:
        module = code_map[name]
        func = getattr(module,func)
        func(*args)
        ret = 0
    except Exception as e:
        log.exception(e)
    return ret





