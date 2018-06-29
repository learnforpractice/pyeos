#include <eosiolib/multi_index.hpp>
using namespace std;

struct boost_account {
   account_name    account;

   uint64_t primary_key()const { return account; }

   EOSLIB_SERIALIZE( boost_account, (account) )
};

bool is_boost_account(uint64_t account) {
   eosio::multi_index<N(boost), boost_account> boost(N(eosio), N(eosio));
   if (boost.find(account) != boost.end()) {
      return true;
   }
   return false;
}

