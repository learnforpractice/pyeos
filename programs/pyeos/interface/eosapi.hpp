#include <Python.h>
#include <string>
#include <vector>
#include <stdint.h>

using namespace std;

class eosapi {
public:
   eosapi();
   virtual ~eosapi();
   virtual PyObject* create_account_(string creator, string newaccount, string owner, string active, int sign);
   virtual PyObject* get_info_();
   virtual PyObject* get_block_(char* num_or_id);
   virtual PyObject* get_account_(const char* _name);
   virtual PyObject* get_currency_balance_(string& _code, string& _account, string& _symbol);
   virtual PyObject* get_actions_(uint64_t account, int pos, int offset);
   virtual PyObject* get_transaction_(string& id);
   virtual PyObject* push_transactions_(vector<vector<chain::action>>& vv, bool sign, uint64_t skip_flag, bool async, bool compress, int32_t max_ram_usage);
   virtual PyObject* gen_transaction_(vector<chain::action>& v, int expiration);
   virtual PyObject* sign_transaction_(string& trx_json_to_sign, string& str_private_key);
   virtual PyObject* push_raw_transaction_(string& signed_trx);
   virtual int get_code_(string& name, string& wast, string& str_abi, string& code_hash, int& vm_type);
   virtual void get_code_hash_(string& name, string& code_hash);
   virtual int get_table_(string& scope, string& code, string& table, string& result);
   virtual void fc_pack_updateauth(string& _account, string& _permission, string& _parent, string& _auth, uint32_t _delay, string& result);
   virtual void fc_pack_args(uint64_t code, uint64_t action, string& json, string& bin);
   virtual PyObject* fc_unpack_args(uint64_t code, uint64_t action, string& bin);
};
