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

template<class Encoder> auto encode(char* data, size_t datalen) {
   Encoder e;
   const size_t bs = eosio::chain::config::hashing_checktime_block_size;
   while ( datalen > bs ) {
      e.write( data, bs );
      data += bs;
      datalen -= bs;
//      ctx().trx_context.checktime();
   }
   e.write( data, datalen );
   return e.result();
}
extern "C" {
void assert_sha256( char* data, uint32_t datalen, const struct checksum256* hash ) {
   FC_ASSERT(data != nullptr && datalen != 0 && hash!= nullptr);
   auto result = encode<fc::sha256::encoder>( data, datalen );
   FC_ASSERT( memcmp(&result, hash, sizeof(*hash)) == 0, "hash mismatch" );
}

void assert_sha1( char* data, uint32_t datalen, const struct checksum160* hash ) {
   FC_ASSERT(data != nullptr && datalen != 0 && hash!= nullptr);
   auto result = encode<fc::sha1::encoder>( data, datalen );
   FC_ASSERT( memcmp(&result, hash, sizeof(*hash)) == 0, "hash mismatch" );
}

void assert_sha512( char* data, uint32_t datalen, const struct checksum512* hash ) {
   FC_ASSERT(data != nullptr && datalen != 0 && hash!= nullptr);
   auto result = encode<fc::sha512::encoder>( data, datalen );
   FC_ASSERT( memcmp(&result, hash, sizeof(struct checksum512)) == 0, "hash mismatch" );
}

void assert_ripemd160( char* data, uint32_t datalen, const struct checksum160* hash ) {
   FC_ASSERT(data != nullptr && datalen != 0 && hash!= nullptr);
   auto result = encode<fc::ripemd160::encoder>( data, datalen );
   FC_ASSERT( memcmp(&result, hash, sizeof(struct checksum160)) == 0, "hash mismatch" );
}

void sha256( char* data, uint32_t datalen, struct checksum256* hash ) {
   FC_ASSERT(data != nullptr && datalen != 0 && hash!= nullptr);
   auto hash_val = encode<fc::sha256::encoder>( data, datalen );
   memcpy(hash, &hash_val._hash, sizeof(struct checksum256));
}

void sha1( char* data, uint32_t datalen, struct checksum160* hash ) {
   FC_ASSERT(data != nullptr && datalen != 0 && hash!= nullptr);
   auto hash_val = encode<fc::sha1::encoder>( data, datalen );
   memcpy(hash, &hash_val._hash, sizeof(struct checksum160));
}

void sha512( char* data, uint32_t datalen, struct checksum512* hash ) {
   FC_ASSERT(data != nullptr && datalen != 0 && hash!= nullptr);
   auto hash_val = encode<fc::sha512::encoder>( data, datalen );
   memcpy(hash, &hash_val._hash, sizeof(struct checksum512));
}

void ripemd160( char* data, uint32_t datalen, struct checksum160* hash ) {
   FC_ASSERT(data != nullptr && datalen != 0 && hash!= nullptr);
   auto hash_val = encode<fc::ripemd160::encoder>( data, datalen );
   memcpy(hash, &hash_val._hash, sizeof(struct checksum160));
}

int recover_key( const struct checksum256* digest, const char* sig, size_t siglen, char* pub, size_t publen ) {
   FC_ASSERT(digest != nullptr && sig != nullptr && siglen != 0 && pub != nullptr && publen != 0);

   fc::crypto::signature s;
   datastream<const char*> ds( sig, siglen );
   datastream<char*> pubds( pub, publen );

   fc::raw::unpack(ds, s);
   fc::raw::pack( pubds, fc::crypto::public_key( s, *(fc::sha256*)digest, false ) );
   return pubds.tellp();
}

void assert_recover_key( const struct checksum256* digest, const char* sig, size_t siglen, const char* pub, size_t publen ) {
   FC_ASSERT(digest != nullptr && sig != nullptr && siglen != 0 && pub != nullptr && publen != 0);

   fc::crypto::signature s;
   fc::crypto::public_key p;
   datastream<const char*> ds( sig, siglen );
   datastream<const char*> pubds( pub, publen );

   fc::raw::unpack(ds, s);
   fc::raw::unpack(pubds, p);

   auto check = fc::crypto::public_key( s, *(fc::sha256*)digest, false );
   FC_ASSERT( check == p, "Error expected key different than recovered key" );
}

}
