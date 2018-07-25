import db
from eoslib import N, read_action

def test_setattr():
    db.__ab = 'abc'
    print(db.__ab)
    pass

def apply(receiver, code, action):
    print('hello,world')
    try:

        test_setattr()
    except Exception as e:
        print('++++++++', e)
