import time
import wallet
import eosapi
import initeos

producer = eosapi.Producer()

print('please make sure you are running the following command before test')
print('./pyeos/pyeos --manual-gen-block --debug -i')

def init():
    if not eosapi.get_account('cache').permissions:
        with producer:
            r = eosapi.create_account('eosio', 'cache', initeos.key1, initeos.key2)
            assert r

    with producer:
        r = eosapi.set_contract('cache','../../programs/pyeos/contracts/cache/cache.py','../../programs/pyeos/contracts/cache/test.abi', 1)
        assert r

counter = 0
def test():
    global counter
    counter += 1
    with producer:
        r = eosapi.push_message('cache','cachetest','hello: ' + str(counter),{'cache':'active'},rawargs=True)
        assert r

