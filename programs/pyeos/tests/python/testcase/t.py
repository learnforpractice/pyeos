import imp
import marshal
import importlib

code = '''
print('hello,worldddddddd')
'''

def t():
    name = 'hello'
    co = compile(code, name, 'exec')
    r = marshal.dumps(co)
    i = 0
    for c in r:
        i+=1
        print('0x%02x,'%(c,), end='')
        if i%16==0:
            print()
    print()
    co = marshal.loads(r)
    module = imp.new_module(name)
    exec(co, module.__dict__)

