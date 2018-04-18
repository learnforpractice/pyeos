/*
 * util_.hpp
 *
 *  Created on: Nov 12, 2017
 *      Author: newworld
 */

#ifndef PROGRAMS_PYEOS_INTERFACE_UTIL__HPP_
#define PROGRAMS_PYEOS_INTERFACE_UTIL__HPP_

#include <eosio/chain/types.hpp>
#include <string>

using namespace std;
using namespace eosio;
using namespace eosio::chain;

namespace python {

#define Name name

int32_t store_(Name scope, Name code, Name table, void* keys, int key_type,
               char* value, uint32_t valuelen);

int32_t update_(Name scope, Name code, Name table, void* keys, int key_type,
                char* value, uint32_t valuelen);

int32_t remove_(Name scope, Name code, Name table, void* keys, int key_type,
                char* value, uint32_t valuelen);

int32_t load_(Name scope, Name code, Name table, void* keys, int key_type,
              int scope_index, char* value, uint32_t valuelen);

int32_t front_(Name scope, Name code, Name table, void* keys, int key_type,
               int scope_index, char* value, uint32_t valuelen);

int32_t back_(Name scope, Name code, Name table, void* keys, int key_type,
              int scope_index, char* value, uint32_t valuelen);

int32_t next_(Name scope, Name code, Name table, void* keys, int key_type,
              int scope_index, char* value, uint32_t valuelen);

int32_t previous_(Name scope, Name code, Name table, void* keys, int key_type,
                  int scope_index, char* value, uint32_t valuelen);

int32_t lower_bound_(Name scope, Name code, Name table, void* keys,
                     int key_type, int scope_index, char* value,
                     uint32_t valuelen);

int32_t upper_bound_(Name scope, Name code, Name table, void* keys,
                     int key_type, int scope_index, char* value,
                     uint32_t valuelen);

}


#endif /* PROGRAMS_PYEOS_INTERFACE_UTIL__HPP_ */
