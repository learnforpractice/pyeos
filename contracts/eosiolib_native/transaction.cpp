/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eosiolib/transaction.h>

#include "wasm_api.h"

void send_deferred(const uint128_t& sender_id, account_name payer, const char *serialized_transaction, size_t size, uint32_t replace_existing) {
   get_wasm_api()->send_deferred(sender_id, payer,serialized_transaction, size, replace_existing);
}

int cancel_deferred(const uint128_t& sender_id) {
   return get_wasm_api()->cancel_deferred(sender_id);
}

size_t read_transaction(char *buffer, size_t size) {
   return get_wasm_api()->read_transaction(buffer, size);
}

size_t transaction_size() {
   return get_wasm_api()->transaction_size();
}

int tapos_block_num() {
   return get_wasm_api()->tapos_block_num();
}

int tapos_block_prefix() {
   return get_wasm_api()->tapos_block_prefix();
}

time expiration() {
   return get_wasm_api()->expiration();
}

int get_action( uint32_t type, uint32_t index, char* buff, size_t size ) {
   return get_wasm_api()->get_action( type, index, buff, size );
}

int get_context_free_data( uint32_t index, char* buff, size_t size ) {
   return get_wasm_api()->get_context_free_data( index, buff, size );
}
