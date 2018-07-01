#include "native.hpp"

extern "C" void vm_unload_account(uint64_t account);

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
    eosio_assert(_boost.find(account) != _boost.end(), "account already accelerated");
   _boost.emplace( account, [&]( auto& p ) {
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

EOSIO_NATIVE_ABI( eosiosystem::system_contract,
     (boost)(cancelboost)
)
