import os
import re
import sys
import imp
import json
import signal
import pickle
import traceback

import rodb as db
import net
import wallet
import eosapi
import debug

from eosapi import *

import pprint

class _safe_key:
    __slots__ = ['obj']
    def __init__(self, obj):
        self.obj = obj
    def __lt__(self, other):
        return False

pprint._safe_key = _safe_key

class Pprint(pprint.PrettyPrinter):
  def _format(self, object, *args, **kwargs):
    if isinstance(object, str):
      if len(object) > 60:
        object = object[:60] + '...'
    return pprint.PrettyPrinter._format(self, object, *args, **kwargs)

pp = Pprint()
pp = pp.pprint

def t1():
    abi_file = '../contracts/eosio.token/eosio.token.abi'
    setabi = eosapi.pack_setabi(abi_file, 'eosio.token')
    eosapi.push_action('eosio', 'setabi', setabi, {'eosio.token':'active'})

def t2():
    msg = {"issuer":"eosio","maximum_supply":"10000000000.0000 EOS"}
    r = eosapi.push_action('eosio.token', 'create', msg, {'eosio.token':'active'})

def t3():
    r = eosapi.push_action('eosio.token','issue',{"to":"eosio","quantity":"1000.0000 EOS","memo":""},{'eosio':'active'})
    assert r
    
def t4():
    a = ''
    bs = bytearray.fromhex(a)
    bs = bytes(bs)
#    print(bs)
    wast = eosapi.wasm2wast(bs)
    print(wast.decode('utf8'))
exports = '''
  (export "_ZeqRK11checksum256S1_" (func $91))
  (export "_ZeqRK11checksum160S1_" (func $92))
  (export "_ZneRK11checksum160S1_" (func $93))
  (export "_ZN5eosio12require_authERKNS_16permission_levelE" (func $94))
  (export "_ZN11eosiosystem15system_contract7onblockEN5eosio15block_timestampEy" (func $95))
  (export "_ZN11eosiosystem15system_contract24update_elected_producersEN5eosio15block_timestampE" (func $98))
  (export "_ZN11eosiosystem15system_contract12claimrewardsERKy" (func $122))
  (export "_ZN11eosiosystem15system_contract11buyrambytesEyym" (func $136))
  (export "_ZN11eosiosystem14exchange_state7convertEN5eosio5assetENS1_11symbol_typeE" (func $138))
  (export "_ZN11eosiosystem15system_contract6buyramEyyN5eosio5assetE" (func $139))
  (export "_ZN11eosiosystem14exchange_state21convert_from_exchangeERNS0_9connectorEN5eosio5assetE" (func $147))
  (export "_ZN11eosiosystem15system_contract7sellramEyx" (func $151))
  (export "_ZN11eosiosystem19validate_b1_vestingEx" (func $153))
  (export "_ZN11eosiosystem15system_contract8changebwEyyN5eosio5assetES2_b" (func $154))
  (export "_ZN11eosiosystem15system_contract12update_votesEyyRKNSt3__16vectorIyNS1_9allocatorIyEEEEb" (func $174))
  (export "_ZN11eosiosystem15system_contract23propagate_weight_changeERKNS_10voter_infoE" (func $176))
  (export "_ZN11eosiosystem15system_contract10delegatebwEyyN5eosio5assetES2_b" (func $201))
  (export "_ZN11eosiosystem15system_contract12undelegatebwEyyN5eosio5assetES2_" (func $202))
  (export "_ZN11eosiosystem15system_contract6refundEy" (func $203))
  (export "_ZN11eosiosystem15system_contract11regproducerEyRKN5eosio10public_keyERKNSt3__112basic_stringIcNS5_11char_traitsIcEENS5_9allocatorIcEEEEt" (func $204))
  (export "_ZN11eosiosystem15system_contract9unregprodEy" (func $208))
  (export "_ZN11eosiosystem10stake2voteEx" (func $210))
  (export "_ZN11eosiosystem15system_contract12voteproducerEyyRKNSt3__16vectorIyNS1_9allocatorIyEEEE" (func $211))
  (export "_ZN11eosiosystem15system_contract8regproxyEyb" (func $212))
  (export "_ZN11eosiosystem14exchange_state19convert_to_exchangeERNS0_9connectorEN5eosio5assetE" (func $215))
  (export "_ZN11eosiosystem15system_contractC2Ey" (func $216))
  (export "_ZN11eosiosystem15system_contract22get_default_parametersEv" (func $221))
  (export "_ZN11eosiosystem15system_contractD2Ev" (func $222))
  (export "_ZN11eosiosystem15system_contract6setramEy" (func $228))
  (export "_ZN11eosiosystem15system_contract9setparamsERKN5eosio21blockchain_parametersE" (func $229))
  (export "_ZN11eosiosystem15system_contract7setprivEyh" (func $230))
  (export "_ZN11eosiosystem15system_contract11rmvproducerEy" (func $231))
  (export "_ZN11eosiosystem15system_contract7bidnameEyyN5eosio5assetE" (func $233))
  (export "_ZN11eosiosystem6native10newaccountEyy" (func $238))
  (export "apply" (func $240))
  (export "fabs" (func $279))
  (export "pow" (func $280))
  (export "sqrt" (func $281))
  (export "scalbn" (func $282))
  (export "memcmp" (func $283))
  (export "strlen" (func $284))
  (export "_ZN5eosio25set_blockchain_parametersERKNS_21blockchain_parametersE" (func $285))
  (export "_ZN5eosio25get_blockchain_parametersERNS_21blockchain_parametersE" (func $286))
  (export "malloc" (func $287))
  (export "free" (func $290))
  '''

exports = '''(export "memory" (memory $0))
 (export "now" (func $now))
 (export "_ZN5eosio12require_authERKNS_16permission_levelE" (func $_ZN5eosio12require_authERKNS_16permission_levelE))
 (export "_ZN11eosiosystem15system_contract7onblockEN5eosio15block_timestampEy" (func $_ZN11eosiosystem15system_contract7onblockEN5eosio15block_timestampEy))
 (export "_ZN11eosiosystem15system_contract24update_elected_producersEN5eosio15block_timestampE" (func $_ZN11eosiosystem15system_contract24update_elected_producersEN5eosio15block_timestampE))
 (export "_ZN11eosiosystem15system_contract12claimrewardsERKy" (func $_ZN11eosiosystem15system_contract12claimrewardsERKy))
 (export "_ZN11eosiosystem15system_contract11buyrambytesEyym" (func $_ZN11eosiosystem15system_contract11buyrambytesEyym))
 (export "_ZN11eosiosystem14exchange_state7convertEN5eosio5assetENS1_11symbol_typeE" (func $_ZN11eosiosystem14exchange_state7convertEN5eosio5assetENS1_11symbol_typeE))
 (export "_ZN11eosiosystem15system_contract6buyramEyyN5eosio5assetE" (func $_ZN11eosiosystem15system_contract6buyramEyyN5eosio5assetE))
 (export "_ZN11eosiosystem14exchange_state21convert_from_exchangeERNS0_9connectorEN5eosio5assetE" (func $_ZN11eosiosystem14exchange_state21convert_from_exchangeERNS0_9connectorEN5eosio5assetE))
 (export "_ZN11eosiosystem15system_contract7sellramEyx" (func $_ZN11eosiosystem15system_contract7sellramEyx))
 (export "_ZN11eosiosystem19validate_b1_vestingEx" (func $_ZN11eosiosystem19validate_b1_vestingEx))
 (export "_ZN11eosiosystem15system_contract8changebwEyyN5eosio5assetES2_b" (func $_ZN11eosiosystem15system_contract8changebwEyyN5eosio5assetES2_b))
 (export "_ZN11eosiosystem15system_contract12update_votesEyyRKNSt3__16vectorIyNS1_9allocatorIyEEEEb" (func $_ZN11eosiosystem15system_contract12update_votesEyyRKNSt3__16vectorIyNS1_9allocatorIyEEEEb))
 (export "_ZN11eosiosystem15system_contract23propagate_weight_changeERKNS_10voter_infoE" (func $_ZN11eosiosystem15system_contract23propagate_weight_changeERKNS_10voter_infoE))
 (export "_ZN11eosiosystem15system_contract10delegatebwEyyN5eosio5assetES2_b" (func $_ZN11eosiosystem15system_contract10delegatebwEyyN5eosio5assetES2_b))
 (export "_ZN11eosiosystem15system_contract12undelegatebwEyyN5eosio5assetES2_" (func $_ZN11eosiosystem15system_contract12undelegatebwEyyN5eosio5assetES2_))
 (export "_ZN11eosiosystem15system_contract6refundEy" (func $_ZN11eosiosystem15system_contract6refundEy))
 (export "_ZN11eosiosystem15system_contract11regproducerEyRKN5eosio10public_keyERKNSt3__112basic_stringIcNS5_11char_traitsIcEENS5_9allocatorIcEEEEt" (func $_ZN11eosiosystem15system_contract11regproducerEyRKN5eosio10public_keyERKNSt3__112basic_stringIcNS5_11char_traitsIcEENS5_9allocatorIcEEEEt))
 (export "_ZN11eosiosystem15system_contract9unregprodEy" (func $_ZN11eosiosystem15system_contract9unregprodEy))
 (export "_ZN11eosiosystem10stake2voteEx" (func $_ZN11eosiosystem10stake2voteEx))
 (export "_ZN11eosiosystem15system_contract12voteproducerEyyRKNSt3__16vectorIyNS1_9allocatorIyEEEE" (func $_ZN11eosiosystem15system_contract12voteproducerEyyRKNSt3__16vectorIyNS1_9allocatorIyEEEE))
 (export "_ZN11eosiosystem15system_contract8regproxyEyb" (func $_ZN11eosiosystem15system_contract8regproxyEyb))
 (export "_ZN11eosiosystem14exchange_state19convert_to_exchangeERNS0_9connectorEN5eosio5assetE" (func $_ZN11eosiosystem14exchange_state19convert_to_exchangeERNS0_9connectorEN5eosio5assetE))
 (export "_ZN11eosiosystem15system_contractC2Ey" (func $_ZN11eosiosystem15system_contractC2Ey))
 (export "_ZN11eosiosystem15system_contract22get_default_parametersEv" (func $_ZN11eosiosystem15system_contract22get_default_parametersEv))
 (export "_ZN11eosiosystem15system_contractD2Ev" (func $_ZN11eosiosystem15system_contractD2Ev))
 (export "_ZN11eosiosystem15system_contract6setramEy" (func $_ZN11eosiosystem15system_contract6setramEy))
 (export "_ZN11eosiosystem15system_contract9setparamsERKN5eosio21blockchain_parametersE" (func $_ZN11eosiosystem15system_contract9setparamsERKN5eosio21blockchain_parametersE))
 (export "_ZN11eosiosystem15system_contract7setprivEyh" (func $_ZN11eosiosystem15system_contract7setprivEyh))
 (export "_ZN11eosiosystem15system_contract11rmvproducerEy" (func $_ZN11eosiosystem15system_contract11rmvproducerEy))
 (export "_ZN11eosiosystem15system_contract7bidnameEyyN5eosio5assetE" (func $_ZN11eosiosystem15system_contract7bidnameEyyN5eosio5assetE))
 (export "_ZN11eosiosystem6native10newaccountEyy" (func $_ZN11eosiosystem6native10newaccountEyy))
 (export "apply" (func $apply))
 (export "fabs" (func $fabs))
 (export "pow" (func $pow))
 (export "sqrt" (func $sqrt))
 (export "scalbn" (func $scalbn))
 (export "memcmp" (func $memcmp))
 (export "strlen" (func $strlen))
 (export "_ZN5eosio25set_blockchain_parametersERKNS_21blockchain_parametersE" (func $_ZN5eosio25set_blockchain_parametersERKNS_21blockchain_parametersE))
 (export "_ZN5eosio25get_blockchain_parametersERNS_21blockchain_parametersE" (func $_ZN5eosio25get_blockchain_parametersERNS_21blockchain_parametersE))
 (export "malloc" (func $malloc))
 (export "free" (func $free))
 '''

import subprocess
def t5():
    global exports
    exports = exports.split('\n')
    for export in exports:
        export = export.strip().split(' ')
        if len(export) < 3:
            continue
        func = export[1]

        import shlex
        cmds = "c++filt -n "+eval(func)
        cmds = shlex.split(cmds)
        subprocess.call(cmds)

eosio = N('eosio')
setcode = N('setcode')
newaccount = N('newaccount')

class JsonStruct(object):
    def __init__(self, js):
        if isinstance(js, bytes):
            js = js.decode('utf8')
            js = json.loads(js)
        if isinstance(js, str):
            js = json.loads(js)

        for key in js:
            value = js[key]
            if isinstance(value, dict):
                self.__dict__[key] = JsonStruct(value)
            elif isinstance(value, list):
                for i in range(len(value)):
                    v = value[i]
                    if isinstance(v, dict):
                        value[i] = JsonStruct(v)
                self.__dict__[key] = value
            else:
                self.__dict__[key] = value

    def __str__(self):
#        return str(self.__dict__)
        return json.dumps(self, default=lambda x: x.__dict__, sort_keys=False, indent=4, separators=(',', ': '))

    def __repr__(self):
        return json.dumps(self, default=lambda x: x.__dict__, sort_keys=False, indent=4, separators=(',', ': '))


#   account_name                     creator;
#   account_name                     name;
total_contracts = {}
total_accounts = {}
def parse_log(num, trx):
#    print('+++++', num, JsonStruct(trx))
    trx = JsonStruct(trx)
    for act in trx.actions:
        '''
        print(num, act.account, act.name)
        args = eosapi.unpack_args(act.account, act.name, bytes.fromhex(act.data))
        pp(args)
        print()
        '''
        if act.name == 'setprods':
            args = eosapi.unpack_args(act.account, act.name, bytes.fromhex(act.data))
            pp(args)
            print()
        continue

        if not act.account == 'eosio':
            continue
        if not act.name in ['newaccount','buyrambytes','delegatebw']:
            print(num, act.account, act.name)
        if act.name == 'regproducer':
            args = eosapi.unpack_args(act.account, act.name, bytes.fromhex(act.data))
            pp(args)
            print()

        if act.name == 'setcode':
            setcode_raw = bytes.fromhex(act.data)
            account, = struct.unpack('Q', setcode_raw[:8])
            account = n2s(account)
            print(num, 'setcode', account)
            if account in total_contracts:
                total_contracts[account].append(num)
            else:
                total_contracts[account] = [num]

        if act.name == 'newaccount':
            raw_newaccount = bytes.fromhex(act.data)
            creator, name = struct.unpack('QQ', raw_newaccount[:16])
            creator, name = n2s(creator), n2s(name)
            total_accounts[name] = creator
            if name.startswith('eosio.'):
                print(num, 'newaccount', creator, name)

def t6(s=1, e=300e4):
    _path = 'data-dir.bk/blocks'
    debug.block_log_test(_path, s, e, parse_log)
    print(total_contracts)
    print(len(total_contracts))

def get_block(n):
    _path = 'data-dir/blocks'
    return debug.block_log_get_block(_path, n)


def parse_log2(num, trx):
    trx = JsonStruct(trx)
#    print('+++++', num, JsonStruct(trx))
    for act in trx.actions:
        print(num, act.account, act.name)
        args = eosapi.unpack_args(act.account, act.name, bytes.fromhex(act.data))
        pp(args)
        if not act.account == 'eosio':
            continue
        if act.name == 'setcode':
            setcode_raw = bytes.fromhex(act.data)
            account, = struct.unpack('Q', setcode_raw[:8])
            account = n2s(account)
            print(num, 'setcode', account)
            if account in total_contracts:
                total_contracts[account].append(num)
            else:
                total_contracts[account] = [num]

        if act.name == 'newaccount':
            raw_newaccount = bytes.fromhex(act.data)
            creator, name = struct.unpack('QQ', raw_newaccount[:16])
            creator, name = n2s(creator), n2s(name)
            if name.startswith('eosio.'):
                print(num, 'newaccount', creator, name)

def get_actions(n):
    total_contracts.clear()
    _path = 'data-dir/blocks'
    debug.block_log_test(_path, n, n, parse_log2)
    print(total_contracts)
    print(len(total_contracts))

action_map = {}
def raw_action_cb(num, act):
    account, name = struct.unpack('QQ', act[:16])
    account, name = n2s(account), n2s(name)
    try:
        action_map[account][num%100]
    except KeyError:
        action_map[account] = len(act)

def get_raw_actions(s, e):
    _path = 'data-dir.bk/blocks'
    debug.block_log_get_raw_actions(_path, s, e, raw_action_cb)
    print(action_map)

def acc_test(units):
    acc = debug.acc_new()
    debug.acc_add(acc, 100, 1000, 172800)
    v, c = debug.acc_get(acc)
    print(v, c)

    debug.acc_add(acc, units, 1001, 172800)
    v, c = debug.acc_get(acc)
    print(v, c)


    debug.acc_release(acc)

def acc_test2(value_ex):
    
    a = debug.acc_get_used(value_ex)
    b = 0
    for i in range(1000):
        b = debug.acc_get_used(value_ex+i)
        if a != b:
            print(i)
            break
    print(a, b)



