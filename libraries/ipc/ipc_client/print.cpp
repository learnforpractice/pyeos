/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

extern "C" {

void prints( const char* cstr ) {

}

void prints_l( const char* cstr, uint32_t len) {

}

void printi( int64_t val ) {

}

void printui( uint64_t val ) {

}

void printi128( const int128_t* val ) {

}

void printui128( const uint128_t* val ) {

}

void printsf(float val) {
   // Assumes float representation on native side is the same as on the WASM side

}

void printdf(double val) {
   // Assumes double representation on native side is the same as on the WASM side

}

void printqf(const float128_t* val) {

}

void printn( uint64_t n ) {

}

void printhex( const void* data, uint32_t datalen ) {

}

}
