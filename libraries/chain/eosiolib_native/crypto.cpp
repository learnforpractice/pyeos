/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

struct checksum256 {
   uint8_t hash[32];
};

struct checksum160 {
   uint8_t hash[20];
};

struct checksum512 {
   uint8_t hash[64];
};

void assert_sha256( char* data, uint32_t length, const checksum256* hash ) {
   array_ptr<char> ptr(data);
   eosio_assert(data != nullptr && hash != nullptr);
   crypto_api(ctx()).assert_sha256(ptr, length, *(fc::sha256*)(hash));
}

void assert_sha1( char* data, uint32_t length, const checksum160* hash ) {
   array_ptr<char> ptr(data);
   eosio_assert(data != nullptr && hash != nullptr);
   crypto_api(ctx()).assert_sha1(ptr, length, *(fc::sha1*)(hash));
}

void assert_sha512( char* data, uint32_t length, const checksum512* hash ) {
   array_ptr<char> ptr(data);
   eosio_assert(data != nullptr && hash != nullptr);
   crypto_api(ctx()).assert_sha512(ptr, length, *(fc::sha512*)(hash));
}

void assert_ripemd160( char* data, uint32_t length, const checksum160* hash ) {
   array_ptr<char> ptr(data);
   eosio_assert(data != nullptr && hash != nullptr);
   crypto_api(ctx()).assert_ripemd160(ptr, length, *(fc::ripemd160*)(hash));
}

void sha256( char* data, uint32_t length, checksum256* hash ) {
   array_ptr<char> ptr(data);
   eosio_assert(data != nullptr && hash != nullptr);
   crypto_api(ctx()).sha256(ptr, length, *(fc::sha256*)(hash));
}

void sha1( char* data, uint32_t length, checksum160* hash ) {
   array_ptr<char> ptr(data);
   eosio_assert(data != nullptr && hash != nullptr);
   crypto_api(ctx()).sha1(ptr, length, *(fc::sha1*)(hash));
}

void sha512( char* data, uint32_t length, checksum512* hash ) {
   array_ptr<char> ptr(data);
   eosio_assert(data != nullptr && hash != nullptr);
   crypto_api(ctx()).sha512(ptr, length, *(fc::sha512*)(hash));
}

void ripemd160( char* data, uint32_t length, checksum160* hash ) {
   array_ptr<char> ptr(data);
   eosio_assert(data != nullptr && hash != nullptr);
   crypto_api(ctx()).ripemd160(ptr, length, *(fc::ripemd160*)(hash));
}

int recover_key( const checksum256* digest, const char* sig, size_t siglen, char* pub, size_t publen ) {
   array_ptr<char> sig_ptr((char*)sig);
   array_ptr<char> pub_ptr((char*)pub);

   eosio_assert(digest != nullptr && sig != nullptr && pub != nullptr);
   return crypto_api(ctx()).recover_key(*(fc::sha256*)(digest), sig_ptr, siglen, pub_ptr, publen);
}

void assert_recover_key( const checksum256* digest, const char* sig, size_t siglen, const char* pub, size_t publen ) {
   array_ptr<char> sig_ptr((char*)sig);
   array_ptr<char> pub_ptr((char*)pub);

   eosio_assert(digest != nullptr && sig != nullptr && pub != nullptr);
   return crypto_api(ctx()).assert_recover_key(*(fc::sha256*)(digest), sig_ptr, siglen, pub_ptr, publen);
}
