/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

void prints( const char* cstr ) {
   console_api(ctx()).prints(null_terminated_ptr((char*)cstr));
}

void prints_l( const char* cstr, uint32_t len) {
   console_api(ctx()).prints_l(array_ptr<const char>(cstr), len);
}

void printi( int64_t value ) {
   console_api(ctx()).printi(value);
}

void printui( uint64_t value ) {
   console_api(ctx()).printui(value);
}

void printi128( const int128_t* value ) {
   console_api(ctx()).printi128(*value);
}

void printui128( const uint128_t* value ) {
   console_api(ctx()).printui128(*value);
}

void printsf(float value) {
   console_api(ctx()).printsf(value);
}

void printdf(double value) {
   console_api(ctx()).printdf(value);
}

void printqf(const long double* value) {
   console_api(ctx()).printqf(*(float128_t*)value);
}

void printn( uint64_t name ) {
   console_api(ctx()).printn(name);
}

void printhex( const void* data, uint32_t datalen ) {
   console_api(ctx()).printhex(array_ptr<const char>((char*)data), datalen);
}

