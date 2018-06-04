/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eosiolib/crypto.h>
#include "wasm_api.h"

void assert_sha256( char* data, uint32_t length, const checksum256* hash ) {
   get_wasm_api()->assert_sha256(data, length, hash );
}

void assert_sha1( char* data, uint32_t length, const checksum160* hash ) {
   get_wasm_api()->assert_sha1(data, length, hash );
}

void assert_sha512( char* data, uint32_t length, const checksum512* hash ) {
   get_wasm_api()->assert_sha512(data, length, hash );
}

void assert_ripemd160( char* data, uint32_t length, const checksum160* hash ) {
   get_wasm_api()->assert_ripemd160(data, length, hash );
}

void sha256( char* data, uint32_t length, checksum256* hash ) {
   get_wasm_api()->sha256(data, length, hash );
}

void sha1( char* data, uint32_t length, checksum160* hash ) {
   get_wasm_api()->sha1(data, length, hash );
}

void sha512( char* data, uint32_t length, checksum512* hash ) {
   get_wasm_api()->sha512(data, length, hash );
}

void ripemd160( char* data, uint32_t length, checksum160* hash ) {
   get_wasm_api()->ripemd160(data, length, hash );
}

int recover_key( const checksum256* digest, const char* sig, size_t siglen, char* pub, size_t publen ) {
   return get_wasm_api()->recover_key(digest, sig, siglen, pub, publen);
}

void assert_recover_key( const checksum256* digest, const char* sig, size_t siglen, const char* pub, size_t publen ) {
   get_wasm_api()->assert_recover_key(digest, sig, siglen, pub, publen);
}
