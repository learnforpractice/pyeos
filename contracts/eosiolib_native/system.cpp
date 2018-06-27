/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include <eosiolib/system.h>

#include "vm_api.h"

void  eosio_assert( uint32_t test, const char* msg ) {
   get_vm_api()->eosio_assert( test, msg );
}

void  eosio_assert_message( uint32_t test, const char* msg, uint32_t msg_len ) {
   get_vm_api()->eosio_assert_message( test, msg, msg_len );
}

void  eosio_assert_code( uint32_t test, uint64_t code ) {
   get_vm_api()->eosio_assert_code( test, code );
}

void  eosio_exit( int32_t code ) {
   get_vm_api()->eosio_exit( code );
}

uint64_t  current_time() {
   return get_vm_api()->current_time();
}

uint32_t  now() {
   return (uint32_t)( current_time() / 1000000 );
}

uint64_t wasm_call(const char*func, uint64_t* args , int argc) {
   printf("++++++++wasm_call: %s\n", func);
   return get_vm_api()->wasm_call(func, args , argc);
}

