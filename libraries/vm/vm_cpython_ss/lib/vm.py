import db
import eoslib
import struct
import inspector

for _dict in [db.__dict__, eoslib.__dict__, struct.__dict__, int.__dict__]:
    for k in _dict:
        v = _dict[k]
        if callable(v):
#            print('+++++++++',v)
            inspector.add_function_to_white_list(v)

whitelist = [    str,
                 pow,
                 ''.format,
                 list, dict, int,
                 int.from_bytes,
                 int.to_bytes,
                 hasattr,
                 getattr,
                 setattr,
                 print,
                 __build_class__,
             ]

print('++++++++++++++int.from_bytes:', int.from_bytes)
print('++++++++++++++str', str)

for bltin in whitelist:
    inspector.add_function_to_white_list(bltin)

def debug(func):
    def func_wrapper(*args, **kwargs):
        print('vm', func, 'enter')
        ret = func(*args, **kwargs)
        print('vm', func, 'leave')
        return ret
    return func_wrapper

def load_module(co, module_dict):
    inspector.builtin_exec(co, module_dict, module_dict)

def apply(mod, receiver, account, action):
    mod.apply(receiver, account, action)
    return 1

def call(mod, func):
    mod.call(func)
