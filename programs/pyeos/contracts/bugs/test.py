import time
import wallet
import eosapi
import initeos

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init():
    if not eosapi.get_account('bugs').permissions:
        with producer:
            r = eosapi.create_account('eosio', 'bugs', initeos.key1, initeos.key2)
            assert r

    with producer:
        r = eosapi.set_contract('bugs','../../programs/pyeos/contracts/bugs/bugs.py','../../programs/pyeos/contracts/bugs/bugs.abi', 1)
        assert r
    eosapi.produce_block()

def t():
    with producer:
        r = eosapi.push_message('bugs','t1','',{'bugs':'active'},rawargs=True)
        assert r
    eosapi.produce_block()

#test deeply recursive generators
def t2():
    with producer:
        r = eosapi.push_message('bugs','t2','',{'bugs':'active'},rawargs=True)
        assert r
    eosapi.produce_block()

