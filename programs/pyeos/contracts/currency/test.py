import time
import wallet
import eosapi
import initeos

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init(wasm = False):
    with producer:
        if not eosapi.get_account('currency').permissions:
                r = eosapi.create_account('eosio', 'currency', initeos.key1, initeos.key2)
                assert r
        if not eosapi.get_account('test').permissions:
            if not eosapi.get_account('test'):
                r = eosapi.create_account('eosio', 'test', initeos.key1, initeos.key2)
                assert r

    with producer:
        if wasm:
            r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
            assert r
        else:
            r = eosapi.set_contract('currency','../../programs/pyeos/contracts/currency/currency.py','../../contracts/currency/currency.abi', 1)
            assert r

#eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)


def test_issue():
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
def test_create():
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
def test_transfer():
    args = {"from":"currency","to":"eosio","quantity":"20.0000 CUR","memo":"my first transfer"}
    with producer:
        r = eosapi.push_message('currency','transfer',args,{'currency':'active'})
        assert r

def test():
    args = {"to":"currency","quantity":"1000.0000 CUR","memo":""}
    r = eosapi.push_message('currency','issue',args,{'currency':'active'})
    assert r
    eosapi.produce_block()

#'issue',{"to":"currency","quantity":"1000.0000 CUR"
def test2(count):
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

    ret = eosapi.push_messages(contracts, functions, args, per, True, rawargs=False)
    assert ret
    cost = ret['cost_time']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()

def n2s(n, max_digits=4):
    number_map = [None for i in range(10)]
    number_map[0] = '1'
    number_map[1] = '2'
    number_map[2] = '3'
    number_map[3] = '4'
    number_map[4] = '5'
    number_map[5] = 'a'
    number_map[6] = 'b'
    number_map[7] = 'c'
    number_map[8] = 'd'
    number_map[9] = 'e'
    _num = ''
    for i in range(max_digits-1, -1, -1):
        _num += number_map[int(n/(10**i) % 10)]
    return _num

def test3(count, d=0):
    for i in range(1, count):
        currency = 'curre'+n2s(i)
#        currency = 'currency'
        if not eosapi.get_account(currency).permissions:
            r = eosapi.create_account('eosio', currency, initeos.key1, initeos.key2)
            assert r
        if d:
            if d==1:
                r = eosapi.set_contract(currency,'../../programs/pyeos/contracts/currency/currency.py','../../contracts/currency/currency.abi', 1)
            elif d == 2:
                r = eosapi.set_contract(currency, '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
            else:
                assert 0

    eosapi.produce_block()

    accounts = []
    functions = []
    args = []
    per = []
    
    for i in range(1, count):
        currency = 'curre'+n2s(i)
        accounts.append(currency)
        per.append({currency:'active'})
        functions.append('issue')
        arg = str(i)
        args.append({"to":currency,"quantity":"1000.0000 CUR","memo":""})
    ret = eosapi.push_messages(accounts, functions, args, per, True, rawargs=False)

    assert ret
    if ret:
        cost = ret['cost_time']
    eosapi.produce_block()

    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))


def test_performance():
    import wallet
    import struct
    from eoslib import N
    from eostypes import PySignedTransaction, PyMessage
    import time
    
    ts = PySignedTransaction()
    ts.reqire_scope(b'test')
    ts.reqire_scope(b'currency')

    data = struct.pack("QQQ", N(b'currency'), N(b'test'), 1)

    for i in range(900):
        msg = PyMessage()
        msg.init(b'currency', b'transfer', [[b'currency',b'active']], data)
        ts.add_message(msg)

    print('++++ push_transaction2')
    start = time.time()
    eosapi.push_transaction2(ts, True)
    print('cost:',time.time() - start)

    print('++++ produce_block')
    start = time.time()
    with producer:
        pass
    print('cost:',time.time() - start)

def compare_performance():
    with producer:
        r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
        assert r

    time.sleep(3.0)
    test_performance()

    info = eosapi.get_code('currency')
    with producer:
        r = eosapi.set_contract('currency','../../programs/pyeos/contracts/currency/currency.py','../../contracts/currency/currency.abi', eosapi.py_vm_type)
        assert r

    time.sleep(3.0)
    test_performance()


#    r = eosapi.get_table('test','currency','account')
#    print(r)

