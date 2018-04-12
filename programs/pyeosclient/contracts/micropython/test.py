import time
import wallet
import eosapi
import initeos

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init():
    with producer:
        if not eosapi.get_account('micropython'):
                r = eosapi.create_account('inita', 'micropython', initeos.key1, initeos.key2)
                assert r
        if not eosapi.get_account('test'):
            if not eosapi.get_account('test'):
                r = eosapi.create_account('inita', 'test', initeos.key1, initeos.key2)
                assert r

    with producer:
        r = eosapi.set_contract('micropython','../../programs/pyeos/contracts/micropython/mp.py','../../contracts/currency/currency.abi',2)
#        r = eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)
        assert r

#eosapi.set_contract('currency', '../../build/contracts/currency/currency.wast', '../../build/contracts/currency/currency.abi',0)


