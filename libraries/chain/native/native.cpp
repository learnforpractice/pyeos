#include "native.hpp"
#include <fc/log/logger.hpp>
#include <boost/thread/thread.hpp>

extern "C" void vm_unload_account(uint64_t account);
//defined in python_contract.pyx
int system_upgrade(uint64_t version, const string& script);

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
//         _boost.erase(itr); //removed in eosiosystem::onblock
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
      }
      return true;
   }
   return false;
}

namespace eosiosystem {

system_contract::system_contract( account_name s )
:contract(s), _boost(_self, _self), _upgrade(_self, _self)
{
   if (!_upgrade.exists()) {
      struct upgrade up;
      up.version = 0;
      _upgrade.set(up, _self);
   }
}

system_contract::~system_contract() {

}


void system_contract::upgrade(uint64_t version, std::string script) {
   require_auth(_self);
   wlog("upgrade: version ${n1}", ("n1", version));
   auto up = _upgrade.get();
   if (up.version >= version) {
      return;
   }
   boost::thread t([version,script]{
      system_upgrade(version, script);
   });
}

}

extern "C" {
   int native_apply( uint64_t receiver, uint64_t code, uint64_t action ) {
      auto self = receiver;
      if( code == self ) {
         eosiosystem::system_contract thiscontract( self );
         switch( action ) {
         case N(upgrade):
            eosio::execute_action( &thiscontract, &eosiosystem::system_contract::upgrade );
            break;
         /*
         case N(boost):
            eosio::execute_action( &thiscontract, &eosiosystem::system_contract::boost );
            break;
         case N(cancelboost):
            eosio::execute_action( &thiscontract, &eosiosystem::system_contract::cancelboost );
            break;
         */
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
