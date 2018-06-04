/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

uint32_t read_action_data( void* msg, uint32_t len ) {
   array_ptr<char> ptr((char*)msg);
   return action_api(ctx()).read_action_data(ptr, len);
}

uint32_t action_data_size() {
   return action_api(ctx()).action_data_size();
}

void require_recipient( account_name name ) {
   authorization_api(ctx()).require_recipient(name);
}

void require_auth( account_name name ) {
   authorization_api(ctx()).require_authorization(name);
}

void require_auth2( account_name name, permission_name permission ) {
   authorization_api(ctx()).require_authorization(name, permission);
}

bool has_auth( account_name name ) {
   return authorization_api(ctx()).has_authorization(name);
}

bool is_account( account_name name ) {
   return authorization_api(ctx()).is_account(name);
}

void send_inline(char *serialized_action, size_t size) {
   array_ptr<char> ptr((char*)serialized_action);
   transaction_api(ctx()).send_inline(ptr, size);
}

void send_context_free_inline(char *serialized_action, size_t size) {
   array_ptr<char> ptr((char*)serialized_action);
   transaction_api(ctx()).send_context_free_inline(ptr, size);
}

uint64_t  publication_time() {
   return system_api(ctx()).publication_time();
}

account_name current_receiver() {
   return action_api(ctx()).current_receiver();;
}


