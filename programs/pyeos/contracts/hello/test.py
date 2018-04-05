import time
import wallet
import eosapi
import initeos

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init(mpy=True):
    with producer:
        if not eosapi.get_account('hello').permissions:
            r = eosapi.create_account('eosio', 'hello', initeos.key1, initeos.key2)
            assert r

    with producer:
        if mpy:
            with open('../../programs/pyeos/contracts/hello/hello.mpy', 'wb') as f:
                f.write(eosapi.mp_compile('../../programs/pyeos/contracts/hello/hello.py'))
            r = eosapi.set_contract('hello','../../programs/pyeos/contracts/hello/hello.mpy','../../programs/pyeos/contracts/hello/hello.abi', 1)
        else:
            r = eosapi.set_contract('hello','../../programs/pyeos/contracts/hello/hello.py','../../programs/pyeos/contracts/hello/hello.abi', 1)
#        r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
            assert r

#eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)

def test(name=None):
    with producer:
        if not eosapi.get_account('hello').permissions:
            r = eosapi.create_account('eosio', 'hello', initeos.key1, initeos.key2)
            assert r
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('hello','sayhello',name,{'hello':'active'},rawargs=True)
        assert r

def test2(count):
    import time
    import json
    functions = []
    args = []
    per = {'hello':'active'}
    for i in range(count):
        functions.append('sayhello')
        arg = str(i)
        args.append(arg)
    ret, cost = eosapi.push_messages('hello', functions, args, per, True, rawargs=True)
    assert ret
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()

def deploy_mpy():
    src_dir = '../../programs/pyeos/contracts/hello'
    file_name = 'hello.py'
    
    src_code = eosapi.mp_compile(os.path.join(src_dir, file_name))
    file_name = file_name.replace('.py', '.mpy')
    mod_name = file_name
    msg = int.to_bytes(len(mod_name), 1, 'little')
    msg += mod_name.encode('utf8')
    msg += int.to_bytes(1, 1, 'little') # compiled code
    msg += src_code

    print('++++++++++++++++deply:', file_name)
    r = eosapi.push_message('kitties','deploy',msg,{'kitties':'active'},rawargs=True)
    assert r

    producer.produce_block()


