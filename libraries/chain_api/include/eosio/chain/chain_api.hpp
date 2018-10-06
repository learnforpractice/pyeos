#include <string>
#include <vector>

#include <fc/variant.hpp>
#include <eosio/chain/action.hpp>

using namespace fc;
using namespace std;

namespace eosio { namespace chain {
   class chain_api {
   public:
      chain_api();
      virtual ~chain_api();
      virtual variant create_account_(string creator, string newaccount, string owner, string active, int sign);
      virtual variant get_info_();
      virtual variant get_block_(char* num_or_id);
      virtual variant get_account_(const char* _name);
      virtual variant get_currency_balance_(string& _code, string& _account, string& _symbol);
      virtual variant get_actions_(uint64_t account, int pos, int offset);
      virtual variant get_transaction_(string& id);
      virtual variant push_transactions_(vector<vector<action>>& vv, bool sign, uint64_t skip_flag, bool async, bool compress, int32_t max_ram_usage);
      virtual variant gen_transaction_(vector<action>& v, int expiration);
      virtual variant sign_transaction_(string& trx_json_to_sign, string& str_private_key);
      virtual variant push_raw_transaction_(string& signed_trx);
      virtual int get_code_(string& name, string& wast, string& str_abi, string& code_hash, int& vm_type);
      virtual void get_code_hash_(string& name, string& code_hash);
      virtual int get_table_(string& scope, string& code, string& table, string& result);
      virtual void fc_pack_updateauth(string& _account, string& _permission, string& _parent, string& _auth, uint32_t _delay, string& result);
      virtual void fc_pack_args(uint64_t code, uint64_t action, string& json, string& bin);
      virtual variant fc_unpack_args(uint64_t code, uint64_t action, string& bin);
      virtual string state_dir();
   };
}
}

using namespace eosio::chain;

void register_chain_api(void* api);
chain_api& get_chain_api();
