/*
 * eoslib_.hpp
 *
 *  Created on: Sep 13, 2017
 *      Author: newworld
 */

#ifndef EOSLIB__HPP_
#define EOSLIB__HPP_
#include <eos/types/native.hpp>
#include <string>

using namespace std;
using namespace eos::types;

void requireAuth_(uint64_t account);
int readMessage_(string& buffer);
void requireScope_(uint64_t account);
void requireNotice_(uint64_t account);
uint64_t currentCode_();
uint32_t now_();

uint64_t string_to_uint64_(string str);
string uint64_to_string_(uint64_t n);

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

void pack_(string& raw, string& out);
void unpack_(string& raw, string& out);

#endif /* EOSLIB__HPP_ */
