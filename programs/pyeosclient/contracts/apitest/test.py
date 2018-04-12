import time
import wallet
import eosapi
import initeos

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init():
    if not eosapi.get_account('test').permissions:
        with producer:
            r = eosapi.create_account('eosio', 'test', initeos.key1, initeos.key2)
            assert r

    with producer:
        r = eosapi.set_contract('test','../../programs/pyeos/contracts/apitest/apitest.py','../../programs/pyeos/contracts/apitest/test.abi', 1)
        assert r
    eosapi.produce_block()

def test():
    with producer:
        r = eosapi.push_message('test','dbtest','',{'test':'active','hello':'active'},rawargs=True)
        assert r
    eosapi.produce_block()

