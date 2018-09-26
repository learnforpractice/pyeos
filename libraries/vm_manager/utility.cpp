#include "ro_db.hpp"
#include <eosiolib/multi_index.hpp>
#include <eosio.system/eosio.system.hpp>

#include <fc/time.hpp>

using namespace std;

extern "C" uint64_t  current_time();

struct boost_account {
   account_name    account;
   uint64_t        expiration;

   uint64_t primary_key()const { return account; }

   EOSLIB_SERIALIZE( boost_account, (account)(expiration) )
};

bool is_boost_account(uint64_t account) {
   eosio::multi_index<N(boost), boost_account> _boost_account(N(eosio), N(eosio));
   if (_boost_account.find(account) != _boost_account.end()) {
      return true;
   }
   return false;
}

typedef void (*fn_on_boost_account)(void* v, uint64_t account, uint64_t expiration);

void visit_boost_account(fn_on_boost_account fn, void* param) {
   eosio::multi_index<N(boost), boost_account> _boost_account(N(eosio), N(eosio));
   for (auto itr = _boost_account.begin(); itr != _boost_account.end(); itr++) {
      fn(param, itr->account, itr->expiration);
   }
}


bool is_boost_account(uint64_t account, bool& expired) {
   uint64_t _now = fc::time_point::now().time_since_epoch().count();
   eosio::multi_index<N(boost), boost_account> _boost(N(eosio), N(eosio));

   auto itr = _boost.find(account);
   expired = false;
   if (itr != _boost.end()) {
      if (itr->expiration < _now) {
         expired = true;
      }
      return true;
   }
   return false;
}

using namespace eosiosystem;

void list_producers_() {
   producers_table        _producers(N(eosio), N(eosio));
   auto idx = _producers.get_index<N(prototalvote)>();

   for ( auto it = idx.cbegin(); it != idx.cend() && 0 < it->total_votes && it->active(); ++it ) {
      printf("++++it->owner: %s total votes %f\n", eosio::name{it->owner}.to_string().c_str(), it->total_votes);//it->owner, it->producer_key}, it->location;
   }
}

