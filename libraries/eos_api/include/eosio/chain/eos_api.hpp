/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <string>
using namespace std;

namespace eosio { namespace chain {

//typedef void (*wallet_import_key)(const std::string& name, const std::string& wif_key, bool save);

class eos_api {
public:
   static eos_api& get() {
      static eos_api* instance = nullptr;
      if (!instance) {
         instance = new eos_api();
      }
      return *instance;
   }

//   void wallet_import_key(const std::string& name, const std::string& wif_key, bool save);
   bool (*wallet_import_key)(const std::string& name, const std::string& wif_key, bool save);
   int (*produce_block_start)();
   int (*produce_block_end)();
   int (*produce_block)();
};

} } // namespace eosio::chain

//FC_REFLECT(eosio::chain::apply_context::apply_results, (applied_actions)(deferred_transaction_requests)(deferred_transactions_count))

