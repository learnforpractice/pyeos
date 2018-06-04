/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

void  eosio_assert( uint32_t test, const char* msg ) {
   context_free_system_api(ctx()).eosio_assert(test, null_terminated_ptr((char*)msg));
}

void  eosio_assert_message( uint32_t test, const char* msg, uint32_t msg_len ) {
   context_free_system_api(ctx()).eosio_assert_message(test, array_ptr<const char>(msg), msg_len);
}

void  eosio_assert_code( uint32_t test, uint64_t code ) {
   context_free_system_api(ctx()).eosio_assert_code(test, code);
}

void  eosio_exit( int32_t code ) {
   context_free_system_api(ctx()).eosio_exit(code);
}

uint64_t  current_time() {
   return system_api(ctx()).current_time();
}

uint32_t  now() {
   return (uint32_t)( current_time() / 1000000 );
}

