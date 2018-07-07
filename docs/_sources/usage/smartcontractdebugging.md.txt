# Smart Contract Debugging

Python smart contract support source level debugging. Developers can debug their smart contract in Eclipse IDE with pydevd Eclipse pluging, other IDE such as Visual Studio Code may also be supported, Please search for online resources to find out how to debug Python source remotely. for pydevd, there is a reference from here [manual_adv_remote_debugger](http://www.pydev.org/manual_adv_remote_debugger.html). After the environment has been successfully setting up, run the following command in PyEos console to enable debugging.

```
debug.enable()
```

Set breakpoint at your python smart contract source code in Eclipse. If your Python smart contract source code does not placed in pyeos/contracts directory, them you need to run the following code to specify your source code directory and your smart contract source code file must under directory with the same name.

```
 sys.path.append(<folder where source code directory in>)
```

Deploy your smart contract to the testnet, Use hello contract in pyeos/contracts as example, set breakpoint at hello.py, and run the following code to call hello.py

```
from hello import t
t.test()
```

Smart contract execution will be stopped when the code at the line of breakpoint is being executed.


To disable debugging, run the following code.

```
debug.disable()
```

![Smart Contract Debugging](https://github.com/learnforpractice/pyeos/blob/master/programs/pyeos/debugging/debugging.png)

