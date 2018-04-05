import time
import wallet
import eosapi
import initeos

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init():
    with producer:
        if not eosapi.get_account('currency').permissions:
                r = eosapi.create_account('eosio', 'currency', initeos.key1, initeos.key2)
                assert r
        if not eosapi.get_account('test').permissions:
            if not eosapi.get_account('test'):
                r = eosapi.create_account('eosio', 'test', initeos.key1, initeos.key2)
                assert r

    with producer:
        r = eosapi.set_contract('currency','../../programs/pyeos/contracts/currency/currency.py','../../contracts/currency/currency.abi', 1)
#        r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
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


#'issue',{"to":"currency","quantity":"1000.0000 CUR"
def test2(count):
    import time
    import json
    functions = []
    args = []
    per = {b'currency':b'active'}
    for i in range(count):
        functions.append(b'issue')
        arg = str(i)
        args.append({"to":"currency","quantity":"1000.0000 CUR","memo":""})
    ret, cost = eosapi.push_messages(b'currency', functions, args, per, True, rawargs=False)
    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()

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

