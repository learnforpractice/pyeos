from libcpp.string cimport string
from libcpp.vector cimport vector
import imp
import os
import sys
import logging as log
import tracemalloc
import traceback
import threading

cache_path = './pyeoscache'
code_map = {}
debug_contracts = []
is_stop_at_load_module = False

cdef extern from "<fc/log/logger.hpp>":
    void ilog(string& str)
    ctypedef unsigned long long uint64_t

cdef extern from "<eos/chain/python_interface.hpp>":
    void Py_EnableCodeExecution(int enable, int _only_once)
    void Py_SetWhiteList(const char** _white_list);
    void Py_EnableImportWhiteList(int enable);

    int is_debug_mode();

    int tracemalloc_get_traceback_limit();
    int tracemalloc_is_out_off_memory();
    void tracemalloc_set_max_malloc_size(int nsize);
    int py_tracemalloc_get_max_malloc_size();
    void tracemalloc_set_max_execution_time(int nsize);
    int tracemalloc_get_max_execution_time();

    void call_eos_main();
    void call_start_interactive_console();

cdef extern void stop_tracemalloc():
    tracemalloc.stop()

def add_debug_contract(contract_name):
    global debug_contracts
    if not contract_name in debug_contracts:
        debug_contracts.append(contract_name)

def remove_deubg_contract(contract_name):
    global debug_contracts
    if contract_name in debug_contracts:
        debug_contracts.remove(contract_name)

def show_debug_contracts():
    global debug_contracts
    print(debug_contracts)

def stop_at_load_module(stop):
    global is_stop_at_load_module
    is_stop_at_load_module = stop

cdef extern int python_load(string& name, string& code, string* error):
    global debug_contracts
    global is_stop_at_load_module
    global code_map
    cdef int ret
    cdef string str_error
    
    py_error = None
    ret = 0
    module_name = name
    module_name = module_name.decode('utf8')
#    print('python_load:',name)
    module = code_map.get(module_name)
    cdef bytes code_ = code
    if not module or (module.__code != code_) or is_debug_mode():
        try:
            new_module = None
            if is_debug_mode() and module_name in debug_contracts:
                if not os.path.exists(cache_path):
                    os.mkdir(cache_path)
                file_name = os.path.join(cache_path, module_name+'.py')
                print(file_name)

                if module_name in sys.modules:
                    if os.path.exists(file_name):
                        old_code = None
                        with open(file_name, 'rb') as f:
                            old_code = f.read()
                    new_code = b'import pydevd\n'
                    if is_stop_at_load_module:
                        new_code += b'pydevd.settrace(suspend=True)\n'
                    else:
                        new_code += b'pydevd.settrace(suspend=False)\n'
                    new_code += code
                    if old_code != new_code:
                        with open(file_name, 'wb') as f:
                            f.write(new_code)
                        try:
                            exec('imp.reload({0})'.format(module_name))
                        except Exception as e:
                            print(e)
                exec('import ' + module_name)

                file_name = os.path.abspath(file_name)

                new_module = sys.modules[module_name]
                #new_module = imp.load_source(module_name, file_name)
            else:
                tracemalloc.stop()
                tracemalloc.start()
                Py_EnableCodeExecution(1, 1)
                Py_EnableImportWhiteList(1)

                new_module = imp.new_module(module_name)
                exec(code,vars(new_module))

            if not is_debug_mode():
                Py_EnableImportWhiteList(0)
                Py_EnableCodeExecution(1, 0)

            code_map[module_name] = new_module
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

    module_name = name
    module_name = module_name.decode('utf8')

#    print('python_call:',func)
    try:
        module = code_map[module_name]
        func = getattr(module,func)

        if not is_debug_mode():
            tracemalloc.stop()
            tracemalloc.start()
            
            Py_EnableCodeExecution(0, 0)
            Py_EnableImportWhiteList(1)

        func(*args)

        if not is_debug_mode():
            Py_EnableImportWhiteList(0)
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

def init_smart_contract():
    import eoslib
    import struct
#    import pickle
#    import numpy
    import logging
    tracemalloc.set_max_malloc_size(2000*1024)

def eos_main():
    call_eos_main()

def start_interactive_console():
    call_start_interactive_console()

def start():
    sys.path.insert(0, cache_path)
    init_smart_contract()

    eos_main_thread = threading.Thread(target=eos_main)
    eos_main_thread.start()
    
    print("++++++++++++=start interactive_console")

#    if is_debug_mode():
#       import pydevd
#       pydevd.settrace(suspend=False)

    call_start_interactive_console()
