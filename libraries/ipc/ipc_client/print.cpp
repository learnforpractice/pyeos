/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

extern "C" {

void prints( const char* cstr ) {
   dlog("${n}", ("n", cstr));
}

void prints_l( const char* cstr, uint32_t len) {
   std::string s(cstr, len);
   dlog("${n}", ("n", s));
}

void printi( int64_t val ) {
   dlog("${n}", ("n", val));
}

void printui( uint64_t val ) {
   dlog("${n}", ("n", val));
}

void printi128( const int128_t* val ) {
//   wlog("${n}", ("n", *val));
}

void printui128( const uint128_t* val ) {
//   wlog("${n}", ("n", *val));
}

void printsf(float val) {
   // Assumes float representation on native side is the same as on the WASM side
   dlog("${n}", ("n", val));
}

void printdf(double val) {
   // Assumes double representation on native side is the same as on the WASM side
   dlog("${n}", ("n", val));
}

void printqf(const float128_t* val) {
//   wlog("${n}", ("n", *val));
}

void printn( uint64_t n ) {
   dlog("${n}", ("n", n));
}

void printhex( const void* data, uint32_t datalen ) {
//   std::string s(data, datalen);
//   wlog("${n}", ("n", s));
}

}
