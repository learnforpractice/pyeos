# Creating Your First Python Smart Contract

### Running PyEos

Open a terminal, cd to [PROJECT_DIR]/build/program, run the following command

```
./pyeos/pyeos --manual-gen-block --debug -i
```

If it's the first time you start PyEos, PyEos will create a testing wallet for you, which placed in data-dir/mywallet.wallet, and then console will print the wallet password as below:

```
wallet password: PW5JWE5g6RZ7Fyr2kmCphDqZo4uivdeGpUpndgFZ52rsduhtf9PRJ
```

Since it's for testing only, password will save to data-dir/data.pkl, So next time you start pyeos for testing, pyeos will unlock wallet for you.

Also PyEos will import three private key to the wallet, which is useful for testing.

```
'5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3',
'5JEcwbckBCdmji5j8ZoMHLEUS8TqQiqBG1DRx1X9DN124GUok9s',
'5JbDP55GXN7MLcNYKCnJtfKi9aD2HvHAdY7g8m67zFTAFkY1uBB'
```

Keep in mind that these private key should never be used in real account, otherwise you will lose all of your crypto property in the account.

Beside that, PyEos will create four important accounts for you:

```
eosio.bios, eosio.msig, eosio.system, eosio.token
```

and publish their smart contract on testnet. 

Although the above steps will never happen in the real world, but it's really provide a great convenience for testing smart contract. Thus save a lot of your precious time and make the development more efficient.

### Generating source code with sketch

Run the following command in PyEos console,

```
sketch.build('hello', 'helloworld', 'py')
```

That will create a helloworld directory under your current directory with hello as the testing account name. There are three file generated in the directory:

```
helloworld.py
helloworld.abi
t.py
```

Which helloworld.py is the Python smart contract source code, helloworld.abi is the ABI(Application Binary Interface) file for smart contract, t.py contains code for testing smart contract.

In addition, sketch can also create a wasm smart contract project for you, just type the following code in PyEos console, and the testing process has no difference with Python smart contract.

```
sketch.build('helloworld', 'helloworld', 'cpp')
```

### Testing


Now it's time to run your helloworld smart contract. Type or copy the following command to the PyEos console:

```python
from helloworld import t
t.test()
```

You will see the following output on console in green words:

```
3289633ms thread-1   mpeoslib.cpp:63               print                ] hello,world
```

Congratulations, you have successfully run your first Python smart contract.

Now you can open helloworld.py for coding. Once it's done, just run t.test() again, 
there is no need to run other commands to publish your testing smart contract, the smart contract will be automatically
republish to the testnet if it's been changed during the running of t.test(). You can also edit the testing code in t.py for testing your smart contract. Once it's done, just run t.test() again, there is no need to run reload(t), PyEos will do the magic for you. That also works at the situation of adding a new function in test. 

There are a lot of examples in programs/pyeos/contracts. Some of them are still in develop, if the example throws exception, then it's probably not done yet. Pick up an example you interest in and play with it as you want. 
