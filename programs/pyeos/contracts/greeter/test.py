import time
import wallet
import eosapi
import eoslib
import initeos

from common import init_, producer

def init(func):
    def func_wrapper(*args):
        init_('greeter', 'greeter.py', 'greeter.abi', __file__)
        return func(*args)
    return func_wrapper

@init
def test(name=None):
    with producer:
        if not name:
            name = 'mike'
        r = eosapi.push_message('greeter', 'setgreeting', name, {'greeter':'active'}, rawargs=True)
        assert r

    with producer:
        r = eosapi.push_message('greeter', 'greeting', '' , {'greeter':'active'}, rawargs=True)
        assert r

@init
def test2(count):
    import time
    import json

    contracts = []
    functions = []
    args = []
    per = []
    for i in range(count):
        functions.append('setgreeting')
        arg = str(i)
        args.append(arg)
        contracts.append('greeter')
        per.append({'greeter':'active'})
    ret = eosapi.push_messages(contracts, functions, args, per, True, rawargs=True)
    assert ret
    cost = ret['cost_time']
    print('total cost time:%.3f s, cost per action: %.3f ms, actions per second: %.3f'%(cost/1e6, cost/count/1000, 1*1e6/(cost/count)))
    eosapi.produce_block()

    code = eosapi.N('greeter')
    id = eosapi.hash64('greeting', 0)
    greeting = 0
    itr = eoslib.db_find_i64(code, code, code, id)
    if itr >= 0:
        greeting = eoslib.db_get_i64(itr)
        print(greeting[1:])

@init
def deploy_mpy():
    src_dir = os.path.dirname(os.path.abspath(__file__))
    file_name = 'greeter.py'
    
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


