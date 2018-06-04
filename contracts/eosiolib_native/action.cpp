/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eosiolib/action.h>

uint32_t read_action_data( void* msg, uint32_t len ) {
   return 0;
}

uint32_t action_data_size() {
   return 0;
}

void require_recipient( account_name name ) {
   return;
}

void require_auth( account_name name ) {

}

bool has_auth( account_name name ) {
   return 0;
}

void require_auth2( account_name name, permission_name permission ) {

}

bool is_account( account_name name ) {
   return false;
}

void send_inline(char *serialized_action, size_t size) {

}

void send_context_free_inline(char *serialized_action, size_t size) {

}

void require_write_lock( account_name name ) {

}

void require_read_lock( account_name name ) {

}

uint64_t  publication_time() {
   return 0;
}

account_name current_receiver() {
   return 0;
}

