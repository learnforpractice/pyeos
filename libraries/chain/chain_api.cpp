#include <eosio/chain/chain_api.hpp>
#include <eosio/chain/controller.hpp>

using namespace fc;

namespace eosio { namespace chain {

static controller *s_ctrl = nullptr;
static controller::config *s_cfg = nullptr;

controller& get_chain_controller() {
   return *s_ctrl;
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

int chain_api::get_code_(string& name, string& wast, string& str_abi, string& code_hash, int& vm_type) {
   return 0;
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



}
}
