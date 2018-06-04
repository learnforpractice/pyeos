/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eosiolib/crypto.h>

void assert_sha256( char* data, uint32_t length, const checksum256* hash ) {

}

void assert_sha1( char* data, uint32_t length, const checksum160* hash ) {

}

void assert_sha512( char* data, uint32_t length, const checksum512* hash ) {

}

void assert_ripemd160( char* data, uint32_t length, const checksum160* hash ) {

}

void sha256( char* data, uint32_t length, checksum256* hash ) {

}

void sha1( char* data, uint32_t length, checksum160* hash ) {

}

void sha512( char* data, uint32_t length, checksum512* hash ) {

}

void ripemd160( char* data, uint32_t length, checksum160* hash ) {

}

int recover_key( const checksum256* digest, const char* sig, size_t siglen, char* pub, size_t publen ) {
   return 0;
}

void assert_recover_key( const checksum256* digest, const char* sig, size_t siglen, const char* pub, size_t publen ) {
   return;
}
