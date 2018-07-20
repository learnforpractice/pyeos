/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include "eosio.prods.hpp"
#include <eosio.system/eosio.system.hpp>
namespace eosio {

using namespace eosiosystem;

prods::prods( account_name self ):contract(self),_jit_info(self, self)
{

}

void prods::votejit( account_name producer, account_name account, int64_t last_code_update) {
   require_auth(producer);
   auto it = _jit_info.find(account);
   if (it == _jit_info.end()) {
      _jit_info.emplace(_self, [&]( auto& m ){
         m.account = account;
         m.voted_producers.push_back(producer);
         m.last_code_update = last_code_update;
      });
      return;
   }

   if (it->last_code_update != last_code_update) {
      _jit_info.modify( it, 0, [&]( auto& m ) {
         m.voted_producers.clear();
         m.voted_producers.push_back(producer);
         m.last_code_update = last_code_update;
      });
      return;
   }

   global_state_singleton _global(N(eosio), N(eosio));

   if (!_global.exists()) {
      return;
   }

   {
      if (std::find(it->voted_producers.begin(), it->voted_producers.end(), producer) == it->voted_producers.end()) {
         _jit_info.modify( it, 0, [&]( auto& m ) {
            m.voted_producers.push_back(producer);
            if (m.voted_producers.size() >= _global.get().last_producer_schedule_size/3*2) {
               m.activated = true;
            }
         });
      }
   }
}

void prods::clearjit( account_name producer, account_name account) {
   require_auth(producer);
   auto it = _jit_info.find(account);
   if (it != _jit_info.end()) {
      _jit_info.erase(it);
   }
}

activated_jit_info_table _jit_info(N(eosio.prods), N(eosio.prods));

extern "C" bool is_jit_account_activated(uint64_t account) {
   auto it = _jit_info.find(account);
   if (it == _jit_info.end()) {
      return false;
   }
   return it->activated;
}

extern "C" bool jit_account_deactivate(uint64_t account) {
   auto it = _jit_info.find(account);
   if (it == _jit_info.end()) {
      return false;
   }
   _jit_info.erase(it);
   return true;
}

} /// namespace eosio

EOSIO_ABI( eosio::prods, (votejit)(clearjit) )


