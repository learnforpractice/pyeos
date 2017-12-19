cdef extern from "<eos/chain/python_interface.hpp>":
    void tinypy_run_code_(const char* code)

    void Py_EnableCodeExecution(int enable, int _only_once)
    void Py_SetWhiteList(const char** _white_list);
    void Py_EnableImportWhiteList(int enable);

def tinypy_run_code(code):
    if isinstance(code, str):
        code = code.encode('utf8')
    tinypy_run_code_(code)

def add_debug_contract(contract_name):
    tinypy_run_code("import python_contract;python_contract.add_debug_contract('{0}')".format(contract_name))
    
def remove_debug_contract(contract_name):
    tinypy_run_code("import python_contract;python_contract.remove_debug_contract('{0}')".format(contract_name))

def show_debug_contracts():
    tinypy_run_code("import python_contract;python_contract.show_debug_contracts()")

def stop_at_load_module(stop):
    tinypy_run_code("import python_contract;python_contract.stop_at_load_module('{0}')".format(stop))

def enable_code_execution(enable, only_once):
    Py_EnableCodeExecution(enable, only_once)

def enable_whitelist(enable):
    if enable:
        Py_EnableImportWhiteList(1)
    else:
        Py_EnableImportWhiteList(0)

