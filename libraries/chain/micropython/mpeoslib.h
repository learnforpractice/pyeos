/*
 * eoslib_.hpp
 *
 *  Created on: Sep 13, 2017
 *      Author: newworld
 */

#ifndef EOSLIB__HPP_
#define EOSLIB__HPP_

#ifdef __cplusplus
   extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#include "py/objlist.h"
#include "py/objstringio.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/obj.h"
#include "py/compile.h"


uint32_t now_();
void require_auth_(uint64_t account);
mp_obj_t read_message_();
void require_scope_(uint64_t account);
void require_notice_(uint64_t account);
uint64_t current_code_();

uint64_t string_to_uint64_(const char* str);
mp_obj_t uint64_to_string_(uint64_t n);

#define Name uint64_t

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

mp_obj_t pack_(const char* str, int nsize);
mp_obj_t unpack_(const char* str, int nsize);

//int get_account_balance_(Name account, uint64_t& eos_balance, uint64_t& staked_balance, uint32_t& unstaking_balance, uint32_t& last_unstaking_time);
mp_obj_t get_account_balance_(Name account);
uint64_t get_active_producers_();
mp_obj_t  sha256_(const char* str, int nsize);

#ifdef __cplusplus
   }
#endif


#endif /* EOSLIB__HPP_ */
