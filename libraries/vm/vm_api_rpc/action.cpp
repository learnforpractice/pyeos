/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

uint32_t read_action_data( void* msg, uint32_t buffer_size ) {
   return 0;

}

uint32_t action_data_size() {
   return 0;
}

uint64_t current_receiver() {
   return 0;
}

void require_recipient( uint64_t name ) {

}

void require_auth( uint64_t name ) {

}

void require_auth2( uint64_t name, uint64_t permission ) {

}

bool has_auth( uint64_t name ) {
   return false;
}

bool is_account( uint64_t name ) {
   return false;
}

void send_inline(char *data, size_t data_len) {
   return;
}

void send_context_free_inline(char *data, size_t data_len) {
   return;
}

uint64_t  publication_time() {
   return 0;
}

