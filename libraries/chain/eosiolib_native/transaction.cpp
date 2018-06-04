
void send_deferred(const uint128_t& sender_id, account_name payer, const char *serialized_transaction, size_t size, uint32_t replace_existing) {
   transaction_api(ctx(), false).send_deferred(sender_id, payer, array_ptr<char>((char*)serialized_transaction), size, replace_existing);
}

int cancel_deferred(const uint128_t& sender_id) {
   return transaction_api(ctx(), false).cancel_deferred(sender_id);
}

size_t read_transaction(char *buffer, size_t size) {
   return context_free_transaction_api(ctx()).read_transaction(array_ptr<char>(buffer), size);
}

size_t transaction_size() {
   return context_free_transaction_api(ctx()).transaction_size();
}

int tapos_block_num() {
   return context_free_transaction_api(ctx()).tapos_block_num();
}

int tapos_block_prefix() {
   return context_free_transaction_api(ctx()).tapos_block_prefix();
}

uint32_t expiration() {
   return context_free_transaction_api(ctx()).expiration();
}

int get_action( uint32_t type, uint32_t index, char* buff, size_t size ) {
   return context_free_transaction_api(ctx()).get_action(type, index, array_ptr<char>(buff), size);
}

int get_context_free_data( uint32_t index, char* buff, size_t size ) {
   return context_free_api(ctx()).get_context_free_data(index, array_ptr<char>(buff), size);
}
