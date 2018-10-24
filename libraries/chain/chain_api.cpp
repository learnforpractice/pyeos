#include <eosio/chain/controller.hpp>
#include <eosio/chain/chain_api.hpp>

using namespace fc;

namespace eosio { namespace chain {

static controller *s_ctrl = nullptr;
static controller::config *s_cfg = nullptr;

controller& get_chain_controller() {
   return *s_ctrl;
}

controller::config& get_chain_config() {
   return *s_cfg;
}

void chain_api_init(controller *ctrl, controller::config *cfg) {
   s_ctrl = ctrl;
   s_cfg = cfg;
   register_chain_api(new chain_api());
}

chain_api::chain_api() {

}

chain_api::~chain_api() {

}

variant chain_api::create_account_(string creator, string newaccount, string owner, string active, int sign) {
   return variant();
}

variant chain_api::get_info_() {
   return variant();
}

variant chain_api::get_block_(char* num_or_id) {
   return variant();
}

variant chain_api::get_account_(const char* _name) {
   return variant();
}

variant chain_api::get_currency_balance_(string& _code, string& _account, string& _symbol) {
   return variant();
}

variant chain_api::get_actions_(uint64_t account, int pos, int offset) {
   return variant();
}

variant chain_api::get_transaction_(string& id) {
   return variant();
}

variant chain_api::push_transactions_(vector<vector<chain::action>>& vv, bool sign, uint64_t skip_flag, bool async, bool compress, int32_t max_ram_usage) {
   return variant();
}
variant chain_api::gen_transaction_(vector<chain::action>& v, int expiration) {
   return variant();
}

variant chain_api::sign_transaction_(string& trx_json_to_sign, string& str_private_key) {
   return variant();
}

variant chain_api::push_raw_transaction_(string& signed_trx) {
   return variant();
}

void chain_api::get_code_hash_(string& name, string& code_hash) {

}

int chain_api::get_table_(string& scope, string& code, string& table, string& result) {
   return 0;
}

void chain_api::fc_pack_updateauth(string& _account, string& _permission, string& _parent, string& _auth, uint32_t _delay, string& result) {

}

void chain_api::fc_pack_args(uint64_t code, uint64_t action, string& json, string& bin) {

}

variant chain_api::fc_unpack_args(uint64_t code, uint64_t action, string& bin) {
   return variant();
}

string chain_api::state_dir() {
   return s_cfg->state_dir.string();
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



}
}
