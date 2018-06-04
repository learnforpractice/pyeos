/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eosiolib/action.h>
#include "wasm_api.h"

uint32_t read_action_data( void* msg, uint32_t len ) {
   return get_wasm_api()->read_action_data(msg, len);
}

uint32_t action_data_size() {
   return get_wasm_api()->action_data_size();
}

void require_recipient( account_name name ) {
   return get_wasm_api()->require_recipient( name );
}

void require_auth( account_name name ) {
   get_wasm_api()->require_auth(name);
}

bool has_auth( account_name name ) {
   return get_wasm_api()->has_auth( name );
}

void require_auth2( account_name name, permission_name permission ) {
   get_wasm_api()->require_auth2( name, permission );
}

bool is_account( account_name name ) {
   return get_wasm_api()->is_account( name ) ;
}

void send_inline(char *serialized_action, size_t size) {
   get_wasm_api()->send_inline(serialized_action, size);
}

void send_context_free_inline(char *serialized_action, size_t size) {
   get_wasm_api()->send_context_free_inline(serialized_action, size);
}


uint64_t  publication_time() {
   return get_wasm_api()->publication_time();
}

account_name current_receiver() {
   return get_wasm_api()->current_receiver();
}

