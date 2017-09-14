/*
 * eoslib_.hpp
 *
 *  Created on: Sep 13, 2017
 *      Author: newworld
 */

#ifndef EOSLIB__HPP_
#define EOSLIB__HPP_
#include <string>
using namespace std;

void requireAuth_( uint64_t account );
int readMessage_( string& buffer );
void requireScope_(uint64_t account);
void requireNotice_( uint64_t account );
uint64_t string_to_name_( const char* str );

#endif /* EOSLIB__HPP_ */
