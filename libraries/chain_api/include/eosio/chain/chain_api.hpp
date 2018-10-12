#include <string>
#include <vector>

#include <fc/variant.hpp>
#include <eosio/chain/action.hpp>
#include <eosio/chain/transaction.hpp>
#include <eosio/chain/block.hpp>

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
      virtual void get_code_hash_(string& name, string& code_hash);
      virtual int get_table_(string& scope, string& code, string& table, string& result);
      virtual void fc_pack_updateauth(string& _account, string& _permission, string& _parent, string& _auth, uint32_t _delay, string& result);
      virtual void fc_pack_args(uint64_t code, uint64_t action, string& json, string& bin);
      virtual variant fc_unpack_args(uint64_t code, uint64_t action, string& bin);
      virtual string state_dir();




      virtual void get_account( const name& account_name, variant& out )const;
      virtual void get_code( const name& account_name, bool code_as_wasm, variant& out )const;
      virtual void get_abi( const name& account_name, variant& out )const;
      virtual void get_code_hash( const name& account_name, variant& out )const;
      virtual void get_raw_code_and_abi( const name& account_name, variant& out )const;
      virtual void get_raw_abi( const name& account_name, variant& out )const;


      virtual void abi_json_to_bin( const name& code, const name& action, variant& args, variant& out )const;
      virtual void abi_bin_to_json( const name& code, const name& action, const vector<char>& binargs, variant& out )const;
      virtual void get_required_keys( const variant& transaction, flat_set<public_key_type>& available_keys, variant& out )const;

      virtual void get_transaction_id( const transaction& trx, variant& out )const;
      virtual void get_block(const string& block_num_or_id, variant& out ) const;

      virtual void get_block_header_state( const string& block_num_or_id, variant& out ) const;

      virtual void get_table_by_scope( const name& code, const name& table, const string& lower_bound, const string& upper_bound, uint32_t limit, variant& out )const;

      virtual void get_currency_balance( const name& code, const name& account, optional<string>& symbol, variant& out )const;
      virtual void get_currency_stats( const name& code, const string& symbol, variant& out )const;


      virtual void get_producers( bool json, const string& lower_bound, uint32_t limit, variant& out )const;
      virtual void get_producer_schedule( variant& out )const;

      virtual void push_block( const chain::signed_block& block, variant& out );

      virtual void push_transaction( const variant& params, variant& out );
      virtual void push_transactions( const vector<variant>& params, variant& out );


   };
}
}

using namespace eosio::chain;

void register_chain_api(chain_api* api);
chain_api& get_chain_api();


/*

 */


