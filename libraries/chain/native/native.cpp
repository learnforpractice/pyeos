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
//      system_upgrade(version, script);
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


#include <eosio.token/eosio.token.hpp>

void db_store_i64_in_account( uint64_t code, uint64_t scope, uint64_t table_id, uint64_t payer, uint64_t key, const char* buffer, size_t buffer_size );
void db_update_i64_without_check_code( int iterator, uint64_t payer, const char* buffer, size_t buffer_size );
void db_remove_i64_without_check_code( int iterator );

struct currency_stats {
   asset          supply;
   asset          max_supply;
   account_name   issuer;

   uint64_t primary_key()const { return supply.symbol.name(); }
};

typedef eosio::multi_index<N(stat), currency_stats> stats;

static uint64_t eosio_token = N(eosio.token);
static uint64_t table_id = N(accounts);

void sub_balance( uint64_t owner, uint64_t amount, uint64_t symbol ) {

   int it = db_find_i64(eosio_token, owner, table_id, symbol>>8);
   eosio_assert(it >=0, "no balance found");

   asset balance{0, symbol};
   db_get_i64(it, &balance, sizeof(balance));

   eosio_assert( balance.amount >= amount, "overdrawn balance" );

   if( balance.amount == amount ) {
      db_remove_i64_without_check_code(it);
   } else {
      balance.amount -= amount;
      db_update_i64_without_check_code(it, owner, (char*)&balance, sizeof(balance));
   }
}

void add_balance( uint64_t owner, uint64_t amount, uint64_t symbol, uint64_t ram_payer )
{
   int it = db_find_i64(eosio_token, owner, table_id, symbol>>8);

   asset balance{0, symbol};
   if (it < 0) {
      balance.amount = amount;
      db_store_i64_in_account(eosio_token, owner, table_id, ram_payer, symbol>>8, (char*)&balance, sizeof(balance));
   } else {
      db_get_i64(it, &balance, sizeof(balance));
      balance.amount += amount;
      db_update_i64_without_check_code(it, ram_payer, (char*)&balance, sizeof(balance));
   }
}

int transfer(uint64_t from, uint64_t to, uint64_t amount, uint64_t symbol) {
   try {
      eosio_assert( from != to, "cannot transfer to self" );
      require_auth( from );
      eosio_assert( is_account( to ), "to account does not exist");

      auto sym = symbol>>8;
      stats statstable( eosio_token, sym );
      const auto& st = statstable.get( sym );

      require_recipient( from );
      require_recipient( to );

      asset quantity{(int64_t)amount, symbol};

      eosio_assert( quantity.is_valid(), "invalid quantity" );
      eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
      eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

      sub_balance( from, amount, symbol );
      add_balance( to, amount, symbol, from );
      return 1;
   } catch (...) {
       throw;
   }
   return 0;
}
