#!/bin/ksh
# Capacity Test

if [[ $# != 3 ]]; then
  echo This is a capacity test using eosc
  echo Usage: captest.ksh NumberOfAccounts DurationSeconds Concurrency
  echo
  echo NumberOfAccounts
  echo There only needs to be enough accounts so that the same account isn\'t used in a 3 second time period
  echo So if trasfer transaction can do 3000 per second per eosc instance, then set this number to 4000 to be safe
  echo If this number is 0 then captest.ksh will skip creating accounts
  echo
  echo DurationSeconds
  echo How long would you like this test to run
  echo
  echo Concurrency
  echo How many concurrent eosc instances would you like running at the same time
  echo
  echo example:
  echo   nohup eosd \&
  echo   captest.ksh 10 6 1
  echo
  exit
fi

echo If this process gets stuck, press Ctrl + C as eosd may have had an issue
NumberOfAccounts=$1
[[ $1 == 0 ]] && NumberOfAccounts=$(wc -l trans_accounts_1.txt)
DurationSeconds=$2
Concurrency=$3

echo NumberOfAccounts = $NumberOfAccounts
echo DurationSeconds = $DurationSeconds
echo Concurrency = $Concurrency

# kill previous captest if it is still running
if [[ $(pgrep captest.ksh | wc -l) > 1 ]]; then
  for p in $(pgrep captest.ksh)
  do
    [[ $p == $$ ]] && continue
    kill -9 $p
  done
fi
pkill -9 eosc

rm master_names_list.txt
touch master_names_list.txt

function CreateAccountsAndTransactions
{
  # saw a few of these: might just be me
  # tr: write error
  # tr: write error: Connection reset by peer

  echo Creating $1

  echo Creating Keys
  rm $1 2>/dev/null
  touch $1
  rm keys.txt 2>/dev/null
  touch keys.txt
  rm run.txt 2>/dev/null
  touch run.txt
  for Unused in $(seq $NumberOfAccounts)
  do
    echo create key >> run.txt
  done
  eosc - < run.txt | grep public | sed 's/^.* //' >> keys.txt

  echo Creating Accounts
  rm run.txt 2>/dev/null
  touch run.txt
  for PubKey in $(cat keys.txt)
  do
    Name=$(cat /dev/urandom | tr -dc 'a-z' | fold -w 12 | head -n 1)
    if [[ $(grep $Name master_names_list.txt | wc -l) != 0 ]]; then
      echo Error: Randomly reproduced a pre-existing name
      exit
    fi
    echo $Name >> master_names_list.txt
    echo $Name >> $1
    echo create account inita $Name $PubKey EOS6KdkmwhPyc2wxN9SAFwo2PU2h74nWs7urN1uRduAwkcns2uXsa >/dev/null 2>/dev/null >> run.txt
  done
  eosc - < run.txt >/dev/null

  echo Prepairing Transactions
  rm trans_$1 2>/dev/null
  for Name in $(cat $1)
  do
    echo transfer eos $Name 1 >> trans_$1
  done

}

function RunOneCapTest
{
  ((TrxAttempted=0))
  while :
  do
    ((TrxAttempted += NumberOfAccounts))
    eosc - < $1 >> captest.log 2>&1
    [[ $(date +%s) == $TestStop ]] && break
  done
  echo $TrxAttempted > ${1}.attempt_count
}


# Create Accounts
if [[ $1 != 0 ]]; then
  for Inst in $(seq $Concurrency)
  do
    CreateAccountsAndTransactions accounts_${Inst}.txt
  done
fi


# Begin capacity test
rm captest.log 2>/dev/null
echo Stating Capacity Test $(date "+%Y%m%d_%H%M%S")
TestStop=$(date +%s)
((TestStop=TestStop+DurationSeconds))
for Inst in $(seq $((Concurrency-1)))
do
  RunOneCapTest trans_accounts_${Inst}.txt &
done
RunOneCapTest trans_accounts_${Concurrency}.txt

# Wait for completion
sleep 1
while [[ $(pgrep eosc | wc -l) > 0 ]];
do
  sleep 1
done

# Report the results
((TrxAttempted=0))
for Inst in $(seq $Concurrency)
do
  Tmp=$(awk '{ sum += $1 } END { print sum }' trans_accounts_${Inst}.txt.attempt_count)
  ((TrxAttempted += Tmp))
done

echo End Capacity Test $(date "+%Y%m%d_%H%M%S")
echo ----------------------------------
sleep 1
echo Review captest.log for results of the test
echo There were $(grep transaction_id captest.log | wc -l) successful transactions in $DurationSeconds seconds
echo There were supposed to be $TrxAttempted successful transactions
echo That comes to $(echo "scale=2; $(grep transaction_id captest.log | wc -l) / $DurationSeconds" | bc -l) successful transactions per second
echo That comes to $(echo "scale=2; $TrxAttempted / $DurationSeconds" | bc -l) total transactions per second



