
extern "C" {

void send_deferred(const uint128_t& sender_id, uint64_t payer, const char *data, size_t data_len, uint32_t replace_existing) {
   FC_ASSERT(false, "not implemented!");
}

int cancel_deferred(const uint128_t& val) {
   FC_ASSERT(false, "not implemented!");
   return 0;
}

void _send_deferred(const uint128_t* sender_id, uint64_t payer, const char *data, size_t data_len, uint32_t replace_existing) {
   FC_ASSERT(false, "not implemented!");
}

int _cancel_deferred(const uint128_t* val) {
   FC_ASSERT(false, "not implemented!");
   return 0;
}

size_t read_transaction(char *data, size_t buffer_size) {
   FC_ASSERT(false, "not implemented!");
   return 0;
}

size_t transaction_size() {
   FC_ASSERT(false, "not implemented!");
   return 0;
}

int tapos_block_num() {
   FC_ASSERT(false, "not implemented!");
   return 0;
}

int tapos_block_prefix() {
   FC_ASSERT(false, "not implemented!");
   return 0;
}

uint32_t expiration() {
   FC_ASSERT(false, "not implemented!");
   return 0;
}

int get_action( uint32_t type, uint32_t index, char* buffer, size_t buffer_size ) {
   FC_ASSERT(false, "not implemented!");
   return 0;
}

void assert_privileged() {
   FC_ASSERT(false, "not implemented!");
   return;
}

void assert_context_free() {
   FC_ASSERT(false, "not implemented!");
   return;
}

int get_context_free_data( uint32_t index, char* buffer, size_t buffer_size ) {
   FC_ASSERT(false, "not implemented!");
   return 0;
}

}


