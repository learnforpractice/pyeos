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
using namespace eosio::types;

void requireAuth_(uint64_t account);
int readMessage_(string& buffer);
void requireScope_(uint64_t account);
void requireNotice_(uint64_t account);
uint64_t currentCode_();
uint32_t now_();

uint64_t string_to_uint64_(string str);
string uint64_to_string_(uint64_t n);

#define Name name

int32_t store_(Name scope, Name table, void* keys, int keyslen, int key_type,
               char* value, uint32_t valuelen);

int32_t update_(Name scope, Name table, void* keys, int keyslen, int key_type,
                char* value, uint32_t valuelen);

int32_t remove_(Name scope, Name table, void* keys, int keyslen, int key_type,
                char* value, uint32_t valuelen);

int32_t load_(Name scope, Name code, Name table, void* keys, int keyslen, int key_type,
              int scope_index, char* value, uint32_t valuelen);

int32_t front_(Name scope, Name code, Name table, void* keys, int keyslen, int key_type,
               int scope_index, char* value, uint32_t valuelen);

int32_t back_(Name scope, Name code, Name table, void* keys, int keyslen, int key_type,
              int scope_index, char* value, uint32_t valuelen);

int32_t next_(Name scope, Name code, Name table, void* keys, int keyslen, int key_type,
              int scope_index, char* value, uint32_t valuelen);

int32_t previous_(Name scope, Name code, Name table, void* keys, int keyslen, int key_type,
                  int scope_index, char* value, uint32_t valuelen);

int32_t lower_bound_(Name scope, Name code, Name table, void* keys, int keyslen,
                     int key_type, int scope_index, char* value,
                     uint32_t valuelen);

int32_t upper_bound_(Name scope, Name code, Name table, void* keys, int keyslen,
                     int key_type, int scope_index, char* value,
                     uint32_t valuelen);

void pack_(string& raw, string& out);
void unpack_(string& raw, string& out);

int get_account_balance_(Name account, uint64_t& eos_balance, uint64_t& staked_balance, uint32_t& unstaking_balance, uint32_t& last_unstaking_time);
uint64_t get_active_producers_();
void  sha256_(string& data, string& hash);


#endif /* EOSLIB__HPP_ */
