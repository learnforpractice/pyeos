#include "context_free_db.hpp"
#include <eosiolib/multi_index.hpp>

using namespace std;

struct boost_account {
   account_name    account;

   uint64_t primary_key()const { return account; }

   EOSLIB_SERIALIZE( boost_account, (account) )
};

bool is_boost_account(uint64_t account) {
   eosio::multi_index<N(boost), boost_account> _boost_account(N(eosio), N(eosio));
   if (_boost_account.find(account) != _boost_account.end()) {
      return true;
   }
   return false;
}

typedef void (*fn_on_boost_account)(void* v, uint64_t account);

void visit_boost_account(fn_on_boost_account fn, void* param) {
   eosio::multi_index<N(boost), boost_account> _boost_account(N(eosio), N(eosio));
   for (auto itr = _boost_account.begin(); itr != _boost_account.end(); itr++) {
      fn(param, itr->account);
   }
}
