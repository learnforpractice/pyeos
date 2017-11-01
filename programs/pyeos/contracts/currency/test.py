import time
import wallet
import eosapi
print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual_gen_block --debug -i')

class Wait(object):
    def __init__(self):
        pass
    
    def produce_block(self):
        for i in range(5):
            ret = eosapi.produce_block()
            if ret == 0:
                break
            time.sleep(1.0)
        count = 0
        while self.num == eosapi.get_info().head_block_num:  # wait for finish of create account
            time.sleep(0.2)
            count += 1
            if count >= 20:
                print('time out')
                return

    def __call__(self):
        self.num = eosapi.get_info().head_block_num
        self.produce_block()

    def __enter__(self):
        self.num = eosapi.get_info().head_block_num
    
    def __exit__(self, type, value, traceback):
        self.produce_block()

wait = Wait()

def init():
    psw = 'PW5Kd5tv4var9XCzvQWHZVyBMPjHEXwMjH1V19X67kixwxRpPNM4J'
    wallet.open('mywallet')
    wallet.unlock('mywallet', psw)
    
    key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
    key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'

    if not eosapi.get_account('currency'):
        with wait:
            r = eosapi.create_account('inita', 'currency', key1, key2)
            assert r
    with wait:
        r = eosapi.set_contract('currency','../../programs/pyeos/contracts/currency/currency.py','../../contracts/currency/currency.abi',1)
        assert r

def test():
    with wait:
        r = eosapi.push_message('currency','transfer','{"from":"currency","to":"inita","amount":1000}',['currency','inita'],{'currency':'active'})
        assert r
    r = eosapi.get_table('inita','currency','account')
    print(r)
