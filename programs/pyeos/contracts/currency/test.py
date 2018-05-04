import pickle
import time
import wallet
import eosapi
import initeos

from common import smart_call, producer

def init(func):
    def func_wrapper(*args, **kwargs):
        if 'wasm' in kwargs and kwargs['wasm']:
            return smart_call('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi', 0, __file__, func, __name__, args)
        else:
            return smart_call('currency', 'currency.py', 'currency.abi', 2, __file__, func, __name__, args, kwargs)
    return func_wrapper

@init
def test_issue(wasm=False):
    with producer:
        r = eosapi.push_message('currency','issue',{"to":"currency","quantity":"1000.0000 CUR","memo":""},{'currency':'active'})
        assert r

'''
account_name           issuer;
asset                  maximum_supply;
// array<char,32>         issuer_agreement_hash;
uint8_t                issuer_can_freeze     = true;
uint8_t                issuer_can_recall     = true;
uint8_t                issuer_can_whitelist  = true;
'''
@init
def test_create(wasm=False):
    args = {"issuer":"currency",
            "maximum_supply":"1000000000.0000 CUR",
            "can_freeze":'1',
            "can_recall":'1',
            "can_whitelist":'1'
            }
    with producer:
        r = eosapi.push_message('currency','create',args,{'currency':'active'})
        assert r
'''
account_name from;
account_name to;
asset        quantity;
string       memo;
'''
@init
def test_transfer(wasm=False):
    args = {"from":"currency","to":"eosio","quantity":"20.0000 CUR","memo":"my first transfer"}
    with producer:
        r = eosapi.push_message('currency','transfer',args,{'currency':'active'})
        assert r
@init
def test(wasm=False):
    args = {"to":"currency","quantity":"1000.0000 CUR","memo":""}
    r = eosapi.push_message('eosio.token','issue',args,{'eosio.token':'active'})
    assert r
    eosapi.produce_block()

def create_keys(n=100):
    keys = {}
    try:
        with open('keys.pkl', 'rb') as f:
            keys = pickle.load(f)
    except Exception as e:
        print(e)

    for i in range(n):
        key_pair = eosapi.create_key()
#        print(key_pair)
        keys[key_pair.public] = key_pair.private

    with open('keys.pkl', 'wb') as f:
        pickle.dump(keys, f)

    exist_keys = wallet.list_keys()
    for pub in keys:
        if pub in exist_keys:
            continue
        priv_key = keys[pub]
#        print(priv_key)
        wallet.import_key('mywallet', priv_key, False)
    wallet.save('mywallet')

def load_keys():
    keys = {}
    try:
        with open('keys.pkl', 'rb') as f:
            keys = pickle.load(f)
    except Exception as e:
        print(e)
    
    print(keys)

    exist_keys = wallet.list_keys()
    for pub in keys:
        if pub in exist_keys:
            continue
        priv_key = keys[pub]
        print(priv_key)
        wallet.import_key('mywallet', priv_key)

#'issue',{"to":"currency","quantity":"1000.0000 CUR"
@init
def test2(count, wasm=False):
    import time
    import json
    
    contracts = []
    functions = []
    args = []
    per = []
    for i in range(count):
        functions.append(b'issue')
        arg = str(i)
        args.append({"to":"currency","quantity":"1000.0000 CUR","memo":""})
        contracts.append(b'currency')
        per.append({b'currency':b'active'})

    ret = eosapi.push_messages(contracts, functions, args, per, True)
    assert ret
    eosapi.produce_block()
    
    cost = ret['cost_time']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

def n2s(n, max_digits=5):
    number_map = ['1', '2', '3', '4', '5', 'a', 'b', 'c' , 'd', 'e']
    _num = []
    for i in range(max_digits-1, -1, -1):
        _num.append(number_map[int(n/(10**i) % 10)])
    return ''.join(_num)

def deploy_contract(currency, contract_type = 0):
    if contract_type <= 0:
        return
    if contract_type==1:
        r = eosapi.set_contract(currency,'../../programs/pyeos/contracts/currency/currency.py','../../contracts/currency/currency.abi', 1)
    elif contract_type == 2:
        with open('../../programs/pyeos/contracts/currency/currency.mpy', 'wb') as f:
            data = eosapi.mp_compile('../../programs/pyeos/contracts/currency/currency.py')
            f.write(data)
        r = eosapi.set_contract(currency,'../../programs/pyeos/contracts/currency/currency.mpy','../../contracts/currency/currency.abi', 1)
    elif contract_type == 3:
        wast = '../../build/contracts/eosio.token/eosio.token.wast'
        key_words = b"hello,world\\00"
        wast = '../../build/contracts/eosio.token/eosio.token.wast'
        with open(wast, 'rb') as f:
            data = f.read()
            #data.find(key_words)
            replace_str = b"%s\\00"%(currency.encode('utf8'),)
            replace_str.zfill(len(key_words))
            #replace key works with custom words to break the effect of code cache mechanism
            data = data.replace(key_words, replace_str)
            with open('currency2.wast', 'wb') as f:
                f.write(data)
        r = eosapi.set_contract(currency, 'currency2.wast', '../../build/contracts/eosio.token/eosio.token.abi',0)
    else:
        assert 0

def test3(count, d=0):
    keys = list(wallet.list_keys().keys())
    for i in range(0, count):
        currency = 'curre'+n2s(i)
#        currency = 'currency'

        key1 = keys[i]
        key2 = keys[10000+i]

        if not eosapi.get_account(currency).permissions:
            r = eosapi.create_account('eosio', currency, key1, key2)
            assert r
        deploy_contract(currency, d)

    eosapi.produce_block()

    time.sleep(0.5)

    accounts = []
    functions = []
    args = []
    per = []
    
    for i in range(0, count):
        currency = 'curre'+n2s(i)
        accounts.append(currency)
        per.append({currency:'active'})
        functions.append('issue')
        arg = str(i)
        args.append({"to":currency,"quantity":"1000.0000 CUD","memo":""})
    ret = eosapi.push_messages(accounts, functions, args, per, True)

    assert ret
    eosapi.produce_block()

    cost = ret['cost_time']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))


def test4(count, d=0):
    keys = list(wallet.list_keys().keys())
    for i in range(0, count):
        currency = 'curre'+n2s(i)
#        currency = 'currency'

        key1 = keys[i]
        key2 = keys[10000+i]

        if not eosapi.get_account(currency).permissions:
            r = eosapi.create_account('eosio', currency, key1, key2)
            assert r
        deploy_contract(currency, d)

    eosapi.produce_block()

    time.sleep(0.5)

    accounts = []
    functions = []
    args = []
    per = []
    
    for i in range(0, count):
        currency = 'curre'+n2s(i)
        accounts.append(currency)
        per.append({currency:'active'})
        functions.append('create')
        arg = str(i)
        msg = {"issuer":"eosio","maximum_supply":"1000000000.0000 EOK","can_freeze":0,"can_recall":0, "can_whitelist":0}
        #{"to":currency,"quantity":"1000.0000 CUR","memo":""}
        args.append(msg)
    cost = eosapi.push_transactions(accounts, functions, args, per, True)

    eosapi.produce_block()

    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))

@init
def create():
    with producer:
        msg = {"issuer":"eosio","maximum_supply":"1000000000.0000 EOS","can_freeze":0,"can_recall":0, "can_whitelist":0}
        r = eosapi.push_message('eosio.token', 'create', msg, {'eosio.token':'active'})
        assert r

@init
def issue():
    with producer:
        r = eosapi.push_message('eosio.token','issue',{"to":"hello","quantity":"1000.0000 EOS","memo":""},{'eosio':'active'})
        assert r

@init
def transfer():
    with producer:
        msg = {"from":"hello", "to":"auction1", "quantity":"100.0000 EOS", "memo":"m"}
        r = eosapi.push_message('eosio.token', 'transfer', msg, {'hello':'active'})
#        r = eosapi.push_message('eosio.token', 'transfer', msg, {'hello':'active', 'auction1':'active'})
        assert r
