/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eosiolib/transaction.h>


void send_deferred(const uint128_t& sender_id, account_name payer, const char *serialized_transaction, size_t size, uint32_t replace_existing) {

}

int cancel_deferred(const uint128_t& sender_id) {
   return 0;
}

size_t read_transaction(char *buffer, size_t size) {
   return 0;
}

size_t transaction_size() {
   return 0;
}

int tapos_block_num() {
   return 0;
}

int tapos_block_prefix() {
   return 0;
}

time expiration() {
   return 0;
}

int get_action( uint32_t type, uint32_t index, char* buff, size_t size ) {
   return 0;
}

int get_context_free_data( uint32_t index, char* buff, size_t size ) {
   return 0;
}
