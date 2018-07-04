from multiprocessing import Process

def f(name):
    print('hello', name)
    with open('abc.txt', 'w') as f:
        f.write("hi, I'm upgrader\n")

def upgrade(version):
    p = Process(target=f, args=(version,))
    p.start()
    p.join()
