Testing with captest
--------------------------------

captest is a capacity test that uses eosc to call transactions against eosd.

Instructions:

1. Compile captest, Run: make
2. Run captest without parameters will show you the input parameters help.
2. Make sure eosc is findable from this folder, either by setting your $PATH or %PATH% environment folder
   to have the path to the eosc folder (prevered method), or by copying eosc to the current folder
3. Stop any previously running instances of eosd
4. Start eosd with a fresh chain
   A. Backup the data-dir config.ini file somewhere
   B. Remove the data-dir folder
   C. Start eosd, it will immediatly exit, having created the data-dir folder
   D. Restore the backup config.ini to the data-dir folder
   E. Start eosd
5. Try a quick test first, to see if you see any successful transactions.
   A. Run: captest 1000 3 1
   B. Review the report that prints out and see if there are (any) sucessful transactions,
      if so, then you know the real test will work.
6. Run: captest 10000 60 4
   Each account can only have 1 transaction per block, which is 3 seconds.
   So for this test, it assumes 10000 accounts are enough to run 1 transaction per account per block.
   If your system performance can do more than this, then you will see 'duplicate' error messages
   in the *trx_results*.txt files, and you will need to increase this 10000 to something bigger.
   The second parameter is how long you want the test to run. It can be any amount of time you choose.
   The 4 should be the number of CPU cores you have, perhaps we will be able to do more, I shall
   update this document as changes are made.


Note the captest.ksh in this folder.
It is the same as captest, parameters and all, simply in a Korn Shell script version we initially made.


