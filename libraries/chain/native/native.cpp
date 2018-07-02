#include "native.hpp"

extern "C" void vm_unload_account(uint64_t account);

using namespace eosiosystem;

bool remove_expired_boost_accounts() {
   std::vector<uint64_t> v;
   uint64_t _now = current_time();
   eosio::multi_index<N(boost), boost_account> _boost(N(eosio), N(eosio));

   for(auto itr=_boost.begin();itr!=_boost.end(); itr++) {
      if (itr->expiration < _now) {
         v.push_back(itr->account);
      }
   }

   for(uint64_t& a: v) {
      auto itr = _boost.find(a);
      if (itr != _boost.end()) {
         vm_unload_account(a);
         _boost.erase(itr);
      }
   }

   return true;
}

bool is_boost_account_expired(uint64_t account) {
   uint64_t _now = current_time();
   eosio::multi_index<N(boost), boost_account> _boost(N(eosio), N(eosio));
   auto itr = _boost.find(account);
   if (itr != _boost.end()) {
      if (itr->expiration < _now) {
         _boost.erase(itr);
         return true;
      }
   }
   return false;
}

bool is_boost_account(uint64_t account, bool& expired) {
   uint64_t _now = current_time();
   eosio::multi_index<N(boost), boost_account> _boost(N(eosio), N(eosio));

   auto itr = _boost.find(account);
   expired = false;
   if (itr != _boost.end()) {
      if (itr->expiration < _now) {
         expired = true;
         _boost.erase(itr);
          jit_bid_singleton _jitbid(N(eosio), N(eosio));
          if (_jitbid.exists()) {
              jit_bid bid = _jitbid.get();
              bid.jit_remains += 1;
              _jitbid.set(bid, 0);
          }
      }
      return true;
   }
   return false;
}

namespace eosiosystem {

system_contract::system_contract( account_name s )
:contract(s), _boost(_self, _self)
{
}

system_contract::~system_contract() {

}

void system_contract::boost(account_name account) {
   require_auth( N(eosio) );
   eosio_assert(is_account(account), "account does not exist");
    eosio_assert(_boost.find(account) == _boost.end(), "account already accelerated");
   _boost.emplace( N(eosio), [&]( auto& p ) {
         p.account = account;
   });
}

void system_contract::cancelboost(account_name account) {
   require_auth( N(eosio) );
   eosio_assert(is_account(account), "account does not exist");
   auto itr = _boost.find(account);
   eosio_assert( itr != _boost.end(), "account not in list" );
   _boost.erase(itr);
   vm_unload_account(account);
}

}

extern "C" {
   int native_apply( uint64_t receiver, uint64_t code, uint64_t action ) {
      auto self = receiver;
      if( code == self ) {
         eosiosystem::system_contract thiscontract( self );
         switch( action ) {
         case N(boost):
            eosio::execute_action( &thiscontract, &eosiosystem::system_contract::boost );
            break;
         case N(cancelboost):
            eosio::execute_action( &thiscontract, &eosiosystem::system_contract::cancelboost );
            break;
         default:
               return 0;
         }
         return 1;
         /* does not allow destructor of thiscontract to run: eosio_exit(0); */
      }
      return 0;
   }
}
/*
EOSIO_NATIVE_ABI( eosiosystem::system_contract,
     (boost)(cancelboost)
)
*/
