/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>

#include <string>


struct activated_jit_info {
   account_name                account = 0;
   bool                        activated  = false;
   int64_t                     last_code_update = 0;
   std::vector<account_name>   voted_producers; // producer ready list

   uint64_t primary_key()const { return account; }

   // explicit serialization macro is not necessary, used here only to improve compilation time
   EOSLIB_SERIALIZE( activated_jit_info, (account)(activated)(last_code_update)(voted_producers) )
};

typedef eosio::multi_index< N(jitinfo), activated_jit_info>  activated_jit_info_table;


namespace eosio {

   using std::string;

   class prods : public contract {
      public:
         prods( account_name self );
         void votejit( account_name producer, account_name account, int64_t last_code_update);
         void clearjit( account_name from, account_name account);
      private:
         activated_jit_info_table _jit_info;
   };

} /// namespace eosio
