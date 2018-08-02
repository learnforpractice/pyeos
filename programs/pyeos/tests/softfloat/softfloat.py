import db
from eoslib import N, read_action

def sayHello():
    print(0.1+0.1+0.1)

def apply(receiver, code, action):
    if action == N('sayhello'):
        sayHello()
    elif action == N('play'):
        play()

