import db
from eoslib import N, read_action

def test_setattr():
    try:
        db.__ab = 'abc'
        print(db.__ab)
    except Exception as e:
        print(e)

    try:
        db.get_i64 = 'abc'
    except Exception as e:
        print(e)

def test_code_object():
    try:
        (lambda fc=(
            lambda n: [
                c for c in 
                    ().__class__.__bases__[0].__subclasses__() 
                    if c.__name__ == n
                ][0]
            ):
            fc("function")(
                fc("code")(
                0,
                0,
                0,
                0,
                0,
                b"KABOOM",
                (None,),
                (),
                (),
                '',
                '',
                1,
                b'',
                (),
                ()
                ),{}
            )()
        )()
    except Exception as e:
        print(e)
'''
code(argcount, kwonlyargcount, nlocals, stacksize, flags, codestring,
         constants, names, varnames, filename, name, firstlineno,
         lnotab[, freevars[, cellvars]])
'''

def test_attr():
    try:
        type(test_attr.__code__)(0, 0, 0, 0, 0, b'', (), (), (), '', '', 1, b'')
    except Exception as e:
        print(e)

    try:
        test_attr.__code__ = None
    except Exception as e:
        print(e)

def test_builtin_types():
    a = {}
    a = dict()
    a[123] = 456
    print(a[123])
    a = [1, 2, 3]
    print(a[1])

def apply(receiver, code, action):
    print('hello,world')
    try:
        test_setattr()
        test_code_object()
        test_attr()
        test_builtin_types()
    except Exception as e:
        print('++++++++', e)
