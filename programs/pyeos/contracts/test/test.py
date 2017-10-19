import time
import wallet
import eosapi

def init():
    psw = 'PW5Kd5tv4var9XCzvQWHZVyBMPjHEXwMjH1V19X67kixwxRpPNM4J'
    wallet.open('mywallet')
    wallet.unlock('mywallet', psw)
    
    key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
    key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
    if not eosapi.get_account('test'):
        r = eosapi.create_account('inita', 'test', key1, key2)
        assert r

    if not eosapi.get_account('test'):
        r = eosapi.create_account('inita', 'test', key1, key2)
        assert r
        num = eosapi.get_info().head_block_num
        while num == eosapi.get_info().head_block_num:  # wait for finish of create account
            time.sleep(0.2)
    if 1:
        r = eosapi.set_contract('test', '../../programs/pyeos/contracts/test/code.py', '../../programs/pyeos/contracts/test/test.abi', 1)
        assert r
    elif False:
        r = eosapi.set_contract('test', './pyeos/contracts/test/test.wast', '../../programs/pyeos/contracts/test/test.abi', 0)
        assert r
    else:
        if not eosapi.get_account('test2'):
            r = eosapi.create_account('inita', 'test2', key1, key2)
            assert r
            num = eosapi.get_info().head_block_num
            while num == eosapi.get_info().head_block_num:  # wait for finish of create account
                time.sleep(0.2)
        r = eosapi.set_contract('test2', './pyeos/contracts/test/test.wast', '../../programs/pyeos/contracts/test/test.abi', 0)
        assert r

'''
psw = 'PW5Kd5tv4var9XCzvQWHZVyBMPjHEXwMjH1V19X67kixwxRpPNM4J'
wallet.open('mywallet')
wallet.unlock('mywallet', psw)

key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
r = eosapi.create_account('inita', 'test2',key1,key2)

r = eosapi.set_contract('test2','./pyeos/contracts/test/test.wast','../../programs/pyeos/contracts/test/test.abi',0)

r = eosapi.create_account('inita', 'test',key1,key2)

r = eosapi.set_contract('test', '../../programs/pyeos/contracts/test/code.py', '../../programs/pyeos/contracts/test/test.abi', 1)


key1 = 'EOS61MgZLN7Frbc2J7giU7JdYjy2TqnfWFjZuLXvpHJoKzWAj7Nst'
key2 = 'EOS5JuNfuZPATy8oPz9KMZV2asKf9m8fb2bSzftvhW55FKQFakzFL'
r = eosapi.create_account('inita', 'test2',key1,key2)
r = eosapi.set_contract('test2', './pyeos/contracts/test/test.wast', '../../programs/pyeos/contracts/test/test.abi', 0)


args = {"name": "test","balance": [1,2,3]}
r = eosapi.push_message('test','test',args,['test'],{'test':'active'})

from contracts.test import test;test.init()

eosapi.get_table('test','test','game')

'''

def test():
#    r = eosapi.push_message('test','test',{"name": 'inita',"balance": [88,99,100]},['test'],{'test':'active'})
    if 0:
        args = {"challenger": 'a', "host": 'b', "turn": 'c', "winner": "d", "board": [1, 2, 3]}
        r = eosapi.push_message('test', 'game', args, ['test'], {'test':'active'})
    else:
        a = [i for i in range(256)]
        args = {"challenger": 'b', "host": 'b', "turn": 'c', "winner": "d", "board": a}
        r = eosapi.push_message('test', 'game', args, ['test'], {'test':'active'})

'''
args = {"challenger": 'a',"host": 'b',"turn": 'c',"winner": "d","board": [1,2,3]}
r = eosapi.push_message('test','game',args,['test'],{'test':'active'})

args = {"name": "test","balance": [1,2,3]}
r = eosapi.push_message('test','test',args,['test'],{'test':'active'})

'''

'''
r = eosapi.set_contract('test','../contracts/test/test.wast','../../programs/pyeos/contracts/test/test.abi',0)
args = {"challenger": 'a',"host": 'b',"turn": 'c',"winner": "d","board": [1,2,3]}
r = eosapi.push_message('test','game',args,['test'],{'test':'active'})
'''


    
