#include <eosiolib_native/vm_api.h>
#include <eosiolib/types.hpp>

using namespace eosio;

int wasm_setcode(uint64_t account);
int wasm_apply(uint64_t receiver, uint64_t account, uint64_t act);


static struct vm_api s_api;

void vm_init() {

}

void vm_deinit() {
   printf("vm_wasm finalize\n");
}


void register_vm_api(struct vm_api* api) {
   s_api = *api;
}

struct vm_api* get_vm_api() {
   return &s_api;
}

int setcode(uint64_t account) {
   wasm_setcode(account);
   return 0;
}

int apply(uint64_t receiver, uint64_t account, uint64_t act) {
   wasm_apply(receiver, account, act);
   return 0;
}

void resume_billing_timer() {
   get_vm_api()->resume_billing_timer();
}
void pause_billing_timer() {
   get_vm_api()->pause_billing_timer();
}

const char* get_code( uint64_t receiver, size_t* size ) {
   return get_vm_api()->get_code( receiver, size );
}

int db_api_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return get_vm_api()->rodb_find_i64(code, scope, table, id);
}

int32_t db_api_get_i64_ex( int iterator, uint64_t* primary, char* buffer, size_t buffer_size ) {
   return get_vm_api()->rodb_get_i64_ex(iterator, primary, buffer, buffer_size);
}

const char* db_api_get_i64_exex( int itr, size_t* buffer_size ) {
   return get_vm_api()->rodb_get_i64_exex(itr, buffer_size);
}
