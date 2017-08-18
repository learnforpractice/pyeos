/**
 * capacity test
 **/
#include <string>
#include <string.h>
#include <vector>
#include <functional>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <stdarg.h>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <csignal>

using namespace std;

//------------------------------------------------------------

int NumberOfAccounts;
int DurationSeconds;
int Concurrency;

char *va(char *buff, int size, const char *format, ... );
#define V(s...) va(tb, sizeof(tb), ## s)
char *rem1310(char *s);
void CreateAccountsAndTransactions(const char *accountfile);

//------------------------------------------------------------

void signal_caught(int x)
{   
   cout << "Caught signal, exitting" << endl;
   exit(1);
} 
   
int main (int argc, char *argv[]) 
{  
   signal(SIGINT,  signal_caught);
   signal(SIGTERM, signal_caught);
   char tb[4098];
   
   if(argc != 4)
   {
      cout << "This is a capacity test using eosc" << endl;
      cout << "Usage: captest.ksh NumberOfAccounts DurationSeconds Concurrency" << endl;
      cout << "" << endl;
      cout << "NumberOfAccounts" << endl;
      cout << "There only needs to be enough accounts so that the same account isn't used in a 3 second time period" << endl;
      cout << "So if trasfer transaction can do 3000 per 3 seconds per eosc instance, then set this number to 4000 to be safe" << endl;
      cout << "If this number is 0 then captest.ksh will skip creating accounts" << endl;
      cout << "" << endl;
      cout << "DurationSeconds" << endl;
      cout << "How long would you like this test to run" << endl;
      cout << "" << endl;
      cout << "Concurrency" << endl;
      cout << "How many concurrent eosc instances would you like running at the same time" << endl;
      cout << "" << endl;
      cout << "Example:" << endl;      
      cout << "  First make sure eosc is in the current folder or findable via the path environment variable." << endl;
      cout << "  Then make sure eosd is running." << endl;
      cout << "  Run the following command:" << endl;      
      cout << "  captest 1000 6 1" << endl;
      cout << "" << endl;
      exit(1);
   }
   cout << "If this process gets stuck, press Ctrl + C as eosd or eosc may have had an issue." << endl;
   
   NumberOfAccounts = atoi(argv[1]);
   DurationSeconds = atoi(argv[2]);
   Concurrency = atoi(argv[3]);
   if(NumberOfAccounts == 0)
   {
      char buff[65536];
      FILE *fp = fopen("accounts_1_trx.txt", "rb");
      while(fgets(buff, sizeof(buff), fp))
         NumberOfAccounts++;
      fclose(fp);
   }

   cout << "Clearing text files" << endl;
   for(int iA=1; iA<=Concurrency; ++iA)
   {
      remove(V("accounts_%d_trx_results.txt", iA));
      if(atoi(argv[1]) != 0) // Only remove the *_trx_results.txt, user is re-running the test with the same accounts
      {
         remove(V("accounts_%d_keys.txt", iA));
         remove(V("accounts_%d_pubkeys.txt", iA));
         remove(V("accounts_%d_names.txt", iA));
         remove(V("accounts_%d_account_create_results.txt", iA));
         remove(V("accounts_%d_trx.txt", iA));
      }
   }

   if(atoi(argv[1]) != 0)
      for(int iA=1; iA<=Concurrency; ++iA)
         CreateAccountsAndTransactions(V("accounts_%d", iA));

   cout << "----------------------------------" << endl;
   cout << "Stating Capacity Test" << endl;
   mutex count_mutex;
   long TrxAttempted = 0;
   long TestStop = time(NULL) + DurationSeconds;
   auto RunOneCapTest = [&](string accountfile) {
      signal(SIGINT,  signal_caught);
      signal(SIGTERM, signal_caught);
      char tb[4098];     
      do
      {
         {
            lock_guard<mutex> guard(count_mutex);
            TrxAttempted += NumberOfAccounts;
         }         
         system(V("eosc - < %s_trx.txt >> %s_trx_results.txt 2>&1", accountfile.c_str(), accountfile.c_str()));
      }
      while(time(NULL) < TestStop);
   };

   thread threads[Concurrency+1];
   string filenames[Concurrency+1];
   for(int iA=1; iA<=Concurrency; ++iA)
      {
      filenames[iA] = V("accounts_%d", iA);
      thread tmp(RunOneCapTest, filenames[iA]);
      threads[iA].swap(tmp);
      }   
   this_thread::sleep_for(chrono::seconds(1));
   for(int iA=1; iA<=Concurrency; ++iA)
      threads[iA].join();

   cout << "End Capacity Test" << endl;
   cout << "----------------------------------" << endl;
   
   cout << "Gathering information to report results..." << endl;   
   long SuccessCount = 0;
   for(int iA=1; iA<=Concurrency; ++iA)
   {
      FILE *fp = fopen(V("accounts_%d_trx_results.txt", iA), "rb");      
      char buff[65536];
      while(fgets(buff, sizeof(buff), fp))
      {
         rem1310(buff);
         if(strstr(buff, "transaction_id")) SuccessCount++;
      }
      fclose(fp);
   }
   
   cout << "Feel free to review the .txt files for details on the results." << endl;
   cout << "There were " << SuccessCount << " successful transactions in " << DurationSeconds << " seconds." << endl;
   cout << "There were supposed to be " << TrxAttempted << " successful transactions" << endl;
   cout << "That comes to " << setiosflags(ios::fixed) << setprecision(4) << 
      (double)SuccessCount / (double)DurationSeconds << " successful transactions per second." << endl;
   cout << "That comes to " << setiosflags(ios::fixed) << setprecision(4) << 
      (double)TrxAttempted / (double)DurationSeconds << " total transactions per second" << endl;
   cout << "----------------------------------" << endl;
   
   return 0;
}


//-------------------------------------------------------------


char *va(char *buff, int size, const char *format, ... )
{
   va_list argptr;
   va_start(argptr, format);
   vsnprintf(buff, size, format, argptr);
   va_end(argptr);
   return buff;
}

char *rem1310(char *s)
{   
   char *start = s;
   char *f=s;
   while(*f)
   {
      if(*f!=13 && *f!=10) { *s=*f; s++; }
      f++;
   }
   *s=0;
   return start;
}

void CreateAccountsAndTransactions(const char *accountfile)
{
   char tb[4092];

   cout << "Creating keys into " << accountfile << "_keys.txt" << endl;
   FILE *fp = fopen("run.txt", "wb");
   for(int iA = 0; iA < NumberOfAccounts; ++iA)
      fprintf(fp, "create key\n");
   fclose(fp);
   system(V("eosc - <run.txt >%s_keys.txt", accountfile));

   cout << "Gathering public keys into " << accountfile << "_pubkeys.txt" << endl;
   char buff[65536];
   fp = fopen(V("%s_keys.txt", accountfile), "rb");
   FILE *keys = fopen(V("%s_pubkeys.txt", accountfile), "wb");
   while(fgets(buff, sizeof(buff), fp))
   {
      rem1310(buff);
      if(!(strncmp(buff, "public:", 7) == 0)) continue;
      fprintf(keys, "%s\n", &buff[8]);
   }
   fclose(fp);
   fclose(keys);
   
   cout << "Generating account names into " << accountfile << "_names.txt" << endl;   
   FILE *names = fopen(V("%s_names.txt", accountfile), "wb");
   int count = 0;
   static char name[13] = "testaaaaaaaa";
   function<int(int)> recurse = (std::function<int(int)>)[&](int pos) -> int {
      int ret;
      if(pos >= 12) return 1;
      for(; name[pos]<='z'; ++name[pos])
      {
         if((ret=recurse(pos+1)) == 1)
            {
            if(count <= NumberOfAccounts) 
               fprintf(names, "%s\n", name);
            count++;
            if(count == NumberOfAccounts + 2) return 2;
            }
         if(ret == 2) return 2;
         if(name[pos] == 'z') { name[pos] = 'a'; return 0; }
      }
   };
   recurse(4);
   fclose(names);
   
   cout << "Creating account creation transactions" << endl;
   char aname[256];
   char akey[256];
   fp    = fopen("run.txt", "wb");
   names = fopen(V("%s_names.txt", accountfile), "rb");
   keys  = fopen(V("%s_pubkeys.txt", accountfile), "rb");   
   for(int iA = 0; iA < NumberOfAccounts; ++iA)
   {
      fgets(aname, sizeof(aname), names);
      fgets(akey, sizeof(akey), keys);
      rem1310(aname);
      rem1310(akey);
      fprintf(fp, 
         "create account inita %s %s EOS6KdkmwhPyc2wxN9SAFwo2PU2h74nWs7urN1uRduAwkcns2uXsa\n", 
         aname, akey);
   }
   fclose(fp);
   fclose(names);
   fclose(keys);
   system(V("eosc - <run.txt >%s_account_create_results.txt", accountfile));
   
   cout << "Creating transaction file " << accountfile << "_trx.txt" << endl;
   fp    = fopen(V("%s_trx.txt", accountfile), "wb");   
   names = fopen(V("%s_names.txt", accountfile), "rb");
   for(int iA = 0; iA < NumberOfAccounts; ++iA)
   {
      fgets(aname, sizeof(aname), names);
      rem1310(aname);
      fprintf(fp, 
         "transfer eos %s 1\n",
         aname);
   }
   fclose(fp);
   fclose(names);
}

//-------------------------------------------------------------

