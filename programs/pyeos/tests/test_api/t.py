import os
import re
import sys
import imp
import time
import struct

import debug
import wallet
import eosapi
import initeos
import traceback
from eosapi import N, mp_compile, pack_bytes, pack_setabi, push_transactions
from common import prepare, producer

def init(wasm=True):
    def init_decorator(func):
        def func_wrapper(*args, **kwargs):
            if wasm:
                prepare('testapi', 'testapi.wast', 'testapi.abi', __file__)
                return func(*args, **kwargs)
            else:
                prepare('testapi', 'testapi.py', 'testapi.abi', __file__)
                return func(*args, **kwargs)
        return func_wrapper
    return init_decorator

actions = []
def WASM_TEST_HANDLER(a, b):
    act = debug.wasm_test_action(a, b)
    actions.append(['testapi', eosapi.n2s(act), {'testapi':'active'}, ''])

WASM_TEST_HANDLER_EX = WASM_TEST_HANDLER

def add_test(): 
    actions.clear()
      #test_types
    WASM_TEST_HANDLER('test_types', 'types_size');
    WASM_TEST_HANDLER('test_types', 'char_to_symbol');
    WASM_TEST_HANDLER('test_types', 'string_to_name');
    WASM_TEST_HANDLER('test_types', 'name_class');

    #test_action
#    WASM_TEST_HANDLER('test_action', 'read_action_normal');\\
    '''
    char a; //1
    uint64_t b; //8
    int32_t  c; //4
    '''
    act = debug.wasm_test_action('test_action', 'read_action_normal')
    DUMMY_ACTION_DEFAULT_A = 0x45
    DUMMY_ACTION_DEFAULT_B = 0xab11cd1244556677
    DUMMY_ACTION_DEFAULT_C = 0x7451ae12
    data = struct.pack('B', 0x45)
    data += struct.pack('Q', DUMMY_ACTION_DEFAULT_B)
    data += struct.pack('i', DUMMY_ACTION_DEFAULT_C)
    print(data)
    actions.append(['testapi', eosapi.n2s(act), {'testapi':'active'}, data])
    
    WASM_TEST_HANDLER('test_action', 'read_action_to_0');
    WASM_TEST_HANDLER('test_action', 'read_action_to_64k');
    WASM_TEST_HANDLER_EX('test_action', 'require_notice');
    WASM_TEST_HANDLER('test_action', 'require_auth');
    WASM_TEST_HANDLER('test_action', 'assert_false');
    WASM_TEST_HANDLER('test_action', 'assert_true');
    WASM_TEST_HANDLER('test_action', 'test_current_time');
    WASM_TEST_HANDLER('test_action', 'test_abort');
    WASM_TEST_HANDLER_EX('test_action', 'test_current_receiver');
    WASM_TEST_HANDLER('test_action', 'test_publication_time');
    WASM_TEST_HANDLER('test_action', 'test_assert_code');
    
    #test_print
    WASM_TEST_HANDLER('test_print', 'test_prints');
    WASM_TEST_HANDLER('test_print', 'test_prints_l');
    WASM_TEST_HANDLER('test_print', 'test_printi');
    WASM_TEST_HANDLER('test_print', 'test_printui');
    WASM_TEST_HANDLER('test_print', 'test_printi128');
    WASM_TEST_HANDLER('test_print', 'test_printui128');
    WASM_TEST_HANDLER('test_print', 'test_printn');
    WASM_TEST_HANDLER('test_print', 'test_printsf');
    WASM_TEST_HANDLER('test_print', 'test_printdf');
    WASM_TEST_HANDLER('test_print', 'test_printqf');
    
    #test crypto
    WASM_TEST_HANDLER('test_crypto', 'test_recover_key');
    WASM_TEST_HANDLER('test_crypto', 'test_recover_key_assert_true');
    WASM_TEST_HANDLER('test_crypto', 'test_recover_key_assert_false');
    WASM_TEST_HANDLER('test_crypto', 'test_sha1');
    WASM_TEST_HANDLER('test_crypto', 'test_sha256');
    WASM_TEST_HANDLER('test_crypto', 'test_sha512');
    WASM_TEST_HANDLER('test_crypto', 'test_ripemd160');
    WASM_TEST_HANDLER('test_crypto', 'sha1_no_data');
    WASM_TEST_HANDLER('test_crypto', 'sha256_no_data');
    WASM_TEST_HANDLER('test_crypto', 'sha512_no_data');
    WASM_TEST_HANDLER('test_crypto', 'ripemd160_no_data');
    WASM_TEST_HANDLER('test_crypto', 'sha256_null');
    WASM_TEST_HANDLER('test_crypto', 'assert_sha256_false');
    WASM_TEST_HANDLER('test_crypto', 'assert_sha256_true');
    WASM_TEST_HANDLER('test_crypto', 'assert_sha1_false');
    WASM_TEST_HANDLER('test_crypto', 'assert_sha1_true');
    WASM_TEST_HANDLER('test_crypto', 'assert_sha512_false');
    WASM_TEST_HANDLER('test_crypto', 'assert_sha512_true');
    WASM_TEST_HANDLER('test_crypto', 'assert_ripemd160_false');
    WASM_TEST_HANDLER('test_crypto', 'assert_ripemd160_true');
    
    #test transaction
    WASM_TEST_HANDLER('test_transaction', 'test_tapos_block_num');
    WASM_TEST_HANDLER('test_transaction', 'test_tapos_block_prefix');
    WASM_TEST_HANDLER('test_transaction', 'send_action');
    WASM_TEST_HANDLER('test_transaction', 'send_action_inline_fail');
    WASM_TEST_HANDLER('test_transaction', 'send_action_empty');
    WASM_TEST_HANDLER('test_transaction', 'send_action_large');
    WASM_TEST_HANDLER('test_transaction', 'send_action_recurse');
    WASM_TEST_HANDLER('test_transaction', 'test_read_transaction');
    WASM_TEST_HANDLER('test_transaction', 'test_transaction_size');
    
    WASM_TEST_HANDLER_EX('test_transaction', 'send_transaction');
    WASM_TEST_HANDLER_EX('test_transaction', 'send_transaction_empty');
    WASM_TEST_HANDLER_EX('test_transaction', 'send_transaction_trigger_error_handler');
    WASM_TEST_HANDLER_EX('test_transaction', 'send_transaction_large');
    WASM_TEST_HANDLER_EX('test_transaction', 'send_action_sender');
    WASM_TEST_HANDLER('test_transaction', 'deferred_print');
    WASM_TEST_HANDLER_EX('test_transaction', 'send_deferred_transaction');
    WASM_TEST_HANDLER_EX('test_transaction', 'send_deferred_transaction_replace');
    WASM_TEST_HANDLER('test_transaction', 'send_deferred_tx_with_dtt_action');
    WASM_TEST_HANDLER('test_transaction', 'cancel_deferred_transaction_success');
    WASM_TEST_HANDLER('test_transaction', 'cancel_deferred_transaction_not_found');
    WASM_TEST_HANDLER('test_transaction', 'send_cf_action');
    WASM_TEST_HANDLER('test_transaction', 'send_cf_action_fail');
    WASM_TEST_HANDLER('test_transaction', 'stateful_api');
    WASM_TEST_HANDLER('test_transaction', 'context_free_api');
    WASM_TEST_HANDLER('test_transaction', 'new_feature');
    WASM_TEST_HANDLER('test_transaction', 'active_new_feature');

def ttt():
      '''
      //test_compiler_builtins
      WASM_TEST_HANDLER(test_compiler_builtins, test_multi3);
      WASM_TEST_HANDLER(test_compiler_builtins, test_divti3);
      WASM_TEST_HANDLER(test_compiler_builtins, test_divti3_by_0);
      WASM_TEST_HANDLER(test_compiler_builtins, test_udivti3);
      WASM_TEST_HANDLER(test_compiler_builtins, test_udivti3_by_0);
      WASM_TEST_HANDLER(test_compiler_builtins, test_modti3);
      WASM_TEST_HANDLER(test_compiler_builtins, test_modti3_by_0);
      WASM_TEST_HANDLER(test_compiler_builtins, test_umodti3);
      WASM_TEST_HANDLER(test_compiler_builtins, test_umodti3_by_0);
      WASM_TEST_HANDLER(test_compiler_builtins, test_lshlti3);
      WASM_TEST_HANDLER(test_compiler_builtins, test_lshrti3);
      WASM_TEST_HANDLER(test_compiler_builtins, test_ashlti3);
      WASM_TEST_HANDLER(test_compiler_builtins, test_ashrti3);
      '''

      #test named actions
      # We enforce action name matches action data type name, so name mangling will not work for these tests.
      '''
      if ( action == N(dummy_action) ) {
         test_action::test_dummy_action();
         return;
      }
      '''


      #test chain
      WASM_TEST_HANDLER(test_chain, test_activeprods);

      # test fixed_point
      WASM_TEST_HANDLER(test_fixedpoint, create_instances);
      WASM_TEST_HANDLER(test_fixedpoint, test_addition);
      WASM_TEST_HANDLER(test_fixedpoint, test_subtraction);
      WASM_TEST_HANDLER(test_fixedpoint, test_multiplication);
      WASM_TEST_HANDLER(test_fixedpoint, test_division);
      WASM_TEST_HANDLER(test_fixedpoint, test_division_by_0);

      #test checktime
      WASM_TEST_HANDLER(test_checktime, checktime_pass);
      WASM_TEST_HANDLER(test_checktime, checktime_failure);
      WASM_TEST_HANDLER(test_checktime, checktime_sha1_failure);
      WASM_TEST_HANDLER(test_checktime, checktime_assert_sha1_failure);
      WASM_TEST_HANDLER(test_checktime, checktime_sha256_failure);
      WASM_TEST_HANDLER(test_checktime, checktime_assert_sha256_failure);
      WASM_TEST_HANDLER(test_checktime, checktime_sha512_failure);
      WASM_TEST_HANDLER(test_checktime, checktime_assert_sha512_failure);
      WASM_TEST_HANDLER(test_checktime, checktime_ripemd160_failure);
      WASM_TEST_HANDLER(test_checktime, checktime_assert_ripemd160_failure);

      #test datastream
      WASM_TEST_HANDLER(test_datastream, test_basic);

      #test permission
      WASM_TEST_HANDLER_EX(test_permission, check_authorization);
      WASM_TEST_HANDLER_EX(test_permission, test_permission_last_used);
      WASM_TEST_HANDLER_EX(test_permission, test_account_creation_time);
#@init()
def test():
    if not eosapi.get_account('testapi'):
        eosapi.create_account('eosio', 'testapi', initeos.key1, initeos.key2)
        
    act = debug.wasm_test_action('test_types', 'types_size')

    lib_path = '/Users/newworld/dev/pyeos/build-debug/contracts/test_api/libtest_api_natived.dylib'
    debug.set_debug_contract('testapi', lib_path)

    add_test()
    ret, cost = eosapi.push_actions(actions)
    print(ret)
    print(cost)


 