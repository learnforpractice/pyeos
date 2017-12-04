from libcpp.string cimport string
from libcpp.vector cimport vector
import imp
import logging as log
import tracemalloc
import traceback

cdef extern from "<fc/log/logger.hpp>":
    void ilog(string& str)
    ctypedef unsigned long long uint64_t

cdef extern from "<eos/chain/python_interface.hpp>":
    void Py_EnableCodeExecution(int enable, int _only_once)
    void Py_SetWhiteList(const char** _white_list);
    void Py_EnableImportWhiteList(int enable);

    int tracemalloc_get_traceback_limit();
    int tracemalloc_is_out_off_memory();
    void tracemalloc_set_max_malloc_size(int nsize);
    int py_tracemalloc_get_max_malloc_size();
    void tracemalloc_set_max_execution_time(int nsize);
    int tracemalloc_get_max_execution_time();


code_map = {}

cdef extern void stop_tracemalloc():
    tracemalloc.stop()

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

            Py_EnableCodeExecution(1, 1)
            Py_EnableImportWhiteList(1);
            exec(code,vars(new_module))
            Py_EnableImportWhiteList(0);
            Py_EnableCodeExecution(1, 0)

            code_map[name] = new_module
            new_module.__code = code
            tracemalloc.stop()
        except Exception as e:
            tracemalloc.stop()
            Py_EnableCodeExecution(1, 0)
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
        
        Py_EnableCodeExecution(0, 0)
        Py_EnableImportWhiteList(1);
        func(*args)
        Py_EnableImportWhiteList(0);
        Py_EnableCodeExecution(1, 0)

        tracemalloc.stop()
        ret = 0
    except Exception as e:
        tracemalloc.stop()
        Py_EnableCodeExecution(1, 0)

        py_error = traceback.format_exc()
        ret = -1
#        log.exception(e)
    if py_error:
        error[0] = bytes(py_error,'utf8')
    return ret
