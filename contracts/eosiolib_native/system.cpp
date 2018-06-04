/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include <eosiolib/system.h>

void  eosio_assert( uint32_t test, const char* msg ) {

}

void  eosio_assert_message( uint32_t test, const char* msg, uint32_t msg_len ) {

}

void  eosio_assert_code( uint32_t test, uint64_t code ) {

}

[[noreturn]] void  eosio_exit( int32_t code ) {

}

uint64_t  current_time() {
   return 0;
}

uint32_t  now() {
   return (uint32_t)( current_time() / 1000000 );
}

