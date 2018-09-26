# cython: c_string_type=str, c_string_encoding=ascii
from libcpp.string cimport string
from libcpp cimport bool
from eostypes_ cimport * 
import os
import sys
import traceback
import eosapi

cdef extern from "<eosio/chain/evm_interface.hpp>":
    void evm_test_(string _code, string _data);

cdef extern from "../interface/debug_.hpp":

    void debug_test();

    void run_code_(string code)

    void py_debug_enable_(int enable);
    bool py_debug_enabled_();

    void wasm_debug_enable_(int enable)
    bool wasm_debug_enabled_()

    void set_debug_contract_(string& _account, string& path)
    uint64_t get_debug_contract_();
    int mp_is_account2(string& account)

    void wasm_enable_native_contract_(bool b)
    bool wasm_is_native_contract_enabled_()

    void app_set_debug_mode_(bool d)

    uint64_t wasm_test_action_(const char* cls, const char* method)

    void block_log_test_(string& path, int start_block, int end_block)
    void block_log_get_raw_actions_(string& path, int start, int end);

    object block_log_get_block_(string& path, int block_num);

    bool hash_option_(const char* option);

    uint64_t usage_accumulator_new_();
    void usage_accumulator_add_(uint64_t p, uint64_t units, uint32_t ordinal, uint32_t window_size);
    void usage_accumulator_get_(uint64_t p, uint64_t& value_ex, uint64_t& consumed);
    void usage_accumulator_release_(uint64_t p);

    uint64_t acc_get_used_(uint64_t value_ex);

    void add_trusted_account_(uint64_t account);
    void remove_trusted_account_(uint64_t account);

    int vm_run_script_(const char* str);

    void softfloat_test_();

    bool update_permission_(uint64_t account, const string& owner, const string& active);
    uint64_t string_to_uint64_(string str);
    void list_producers_();
    bool set_proposed_producers_(string& producer, string& public_key);


cdef extern from "<eosiolib_native/vm_api.h>":
    cdef cppclass vm_api:
        int (*sha3)(const char* data, int size, char* result, int size2)
        int (*vm_run_lua_script)(const char* cfg, const char* script);

        void (*vm_set_debug_contract)(uint64_t account, const char* path);
        const char* (*vm_get_debug_contract)(uint64_t* account);

    vm_api* get_vm_api()

def eval(const char* code):
    pass
#    get_mpapi().set_debug_mode(1)
#    get_mpapi().execute_from_str(code)
#    get_mpapi().set_debug_mode(0)

def evm_test(string _code, string _data):
    raise Exception("Fix me please.")
#    evm_test_(_code, _data)

def test():
    debug_test()

def run_code(string code):
    pass
    #run_code_(code)

callback = None
cdef extern int block_on_action(int block, object trx):
    global callback
    if callback:
        try:
            callback(block, trx)
        except:
            traceback.print_exc()
            return 0
    return 1

def  block_log_test(string& path, start, end, cb):
    global callback
    callback = cb
    block_log_test_(path, start, end)

raw_action_cb = None
cdef extern int block_on_raw_action(int block, string act):
    global raw_action_cb
    if raw_action_cb:
        try:
            raw_action_cb(block, <bytes>act)
        except:
            traceback.print_exc()
            return 0
    return 1

def block_log_get_raw_actions(string& path, start, end, cb):
    global raw_action_cb
    raw_action_cb = cb
    block_log_get_raw_actions_(path, start, end);

def block_log_get_block(string& path, int block_num):
    return block_log_get_block_(path, block_num)

def has_option(option):
    return hash_option_(option)

def acc_new():
    return usage_accumulator_new_()

def acc_add(uint64_t p, uint64_t units, uint32_t ordinal, uint32_t window_size):
    return usage_accumulator_add_(p, units, ordinal, window_size)

def acc_get(uint64_t p):
    cdef uint64_t value_ex = 0
    cdef uint64_t consumed = 0
    usage_accumulator_get_(p, value_ex, consumed)
    return (value_ex, consumed)

def acc_release(uint64_t p):
    usage_accumulator_release_(p)

def acc_get_used(value_ex):
    return acc_get_used_(value_ex)

def add_trusted_account(account):
    if not isinstance(account, int):
        account = eosapi.s2n(account)
    add_trusted_account_(account)

def remove_trusted_account(account):
    if not isinstance(account, int):
        account = eosapi.s2n(account)
    remove_trusted_account_(account);

def vm_run_script(_str):
    return vm_run_script_(_str)

def softfloat_test():
    softfloat_test_()

def vm_run_lua_script(cfg, script):
    return get_vm_api()[0].vm_run_lua_script(cfg, script)

def vm_set_debug_contract(int vm_type, const char* path):
        get_vm_api()[0].vm_set_debug_contract(vm_type, path);

def vm_get_debug_contract():
    cdef uint64_t account = 0
    cdef const char* path
    path = get_vm_api()[0].vm_get_debug_contract(&account)
    return (account, path)

def update_permission(account, const string& owner, const string& active):
    cdef uint64_t _account;
    if isinstance(account, str):
        _account = string_to_uint64_(account)
    else:
        _account = account
    return update_permission_(_account, owner, active);

def list_producers():
    list_producers_()

def set_proposed_producers(string& producer, string& public_key):
    return set_proposed_producers_(producer, public_key)

