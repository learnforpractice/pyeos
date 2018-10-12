#include <eosio/chain/chain_api.hpp>
#include <eosio/chain/chain_api.h>

static chain_api *s_api = nullptr;
void register_chain_api(chain_api *api) {
   s_api = api;
}

chain_api& get_chain_api() {
//   EOS_ASSERT(s_api, chain_exception, "chain api not initialized!");
   return *s_api;
}

string get_state_dir() {
   return get_chain_api().state_dir();
}



void chain_api::get_account( const name& account_name, variant& out )const {

}

void chain_api::get_code( const name& account_name, bool code_as_wasm, variant& out )const {

}


void chain_api::get_abi( const name& account_name, variant& out )const {

}

void chain_api::get_code_hash( const name& account_name, variant& out )const  {

}

void chain_api::get_raw_code_and_abi( const name& account_name, variant& out )const  {

}

void chain_api::get_raw_abi( const name& account_name, variant& out )const  {

}


void chain_api::abi_json_to_bin( const name& code, const name& action, variant& args, variant& out )const  {

}

void chain_api::abi_bin_to_json( const name& code, const name& action, const vector<char>& binargs, variant& out )const  {

}

void chain_api::get_required_keys( const variant& transaction, flat_set<public_key_type>& available_keys, variant& out )const  {

}

void chain_api::get_transaction_id( const transaction& trx, variant& out )const  {

}

void chain_api::get_block(const string& block_num_or_id, variant& out ) const  {

}

void chain_api::get_block_header_state( const string& block_num_or_id, variant& out ) const  {

}

void chain_api::get_table_by_scope( const name& code, const name& table, const string& lower_bound, const string& upper_bound, uint32_t limit, variant& out )const  {

}

void chain_api::get_currency_balance( const name& code, const name& account, optional<string>& symbol, variant& out )const  {

}

void chain_api::get_currency_stats( const name& code, const string& symbol, variant& out )const  {

}

void chain_api::get_producer_schedule( variant& out )const  {

}

void chain_api::get_producers( bool json, const string& lower_bound, uint32_t limit, variant& out )const {

}


void chain_api::push_block(const chain::signed_block& block, variant& out) {

}

void chain_api::push_transaction(const variant& params, variant& out) {

}

void chain_api::push_transactions(const vector<variant>& params, variant& out) {

}
