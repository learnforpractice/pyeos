#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <map>
#include <string>
#include <vector>
#include <memory>

#include <vm_py_api.h>
#include <vm_wasm_api.h>

#include <eosio/chain/db_api.hpp>


using namespace std;

namespace eosio {
namespace chain {

#include <eosiolib_native/vm_api.h>

struct vm_calls {
   uint32_t version;
   void* handle;
   fn_vm_init vm_init;
   fn_vm_deinit vm_deinit;
   fn_setcode setcode;
   fn_apply apply;
   fn_preload preload;
   fn_unload unload;
};


class vm_manager
{
public:
   static vm_manager& get();
   void set_vm_api(struct vm_api* _api);
   struct vm_api* get_vm_api();

   int setcode(int type, uint64_t account);
   int apply(int type, uint64_t receiver, uint64_t account, uint64_t act);
   int local_apply(int type, uint64_t receiver, uint64_t account, uint64_t act);

   int check_new_version(int vm_type, uint64_t vm_name);
   int load_vm_from_path(int vm_type, const char* vm_path);
   int load_vm_from_ram(int vm_type, uint64_t vm_name);
   int load_vm(int vm_type);
   int load_vm_wavm();

   bool init(struct vm_api* api);

   struct vm_wasm_api* get_wasm_vm_api();
   struct vm_py_api* get_py_vm_api();
   void *get_eth_vm_api();

   uint64_t wasm_call(const string& func, vector<uint64_t> args);
   void on_boost_account(uint64_t account);
   void preload_accounts(vm_calls* _calls);

   void unload_account(uint64_t account);
   bool is_trusted_account(uint64_t account);
   int vm_deinit_all();

   void add_trusted_account(uint64_t account);
   void remove_trusted_account(uint64_t account);
private:
   vm_manager();
   struct vm_api* api;
   vector<uint64_t> boost_accounts;
   map<uint64_t, uint64_t> trusted_accounts;
   map<int, std::unique_ptr<vm_calls>> vm_map;
   map<uint64_t, std::unique_ptr<vm_calls>> preload_account_map;
};

}
}
