import os
import json

accounts_json = os.path.dirname(__file__)
accounts_json = os.path.join(accounts_json, '../../../..', 'tutorials/bios-boot-tutorial/accounts.json')

with open(accounts_json) as f:
    a = json.load(f)
    firstProducer = len(a['users'])
    numProducers = len(a['producers'])
    accounts = a['users'] + a['producers']
print(accounts[0])
print(len(accounts))

for a in accounts:
    eosapi.create_account('eosio', a['name'], a['pub'], a['pub'])
    