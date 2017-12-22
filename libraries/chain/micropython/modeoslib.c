/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "mpeoslib.h"

#if MICROPY_PY_EOSLIB

// The function below implements a simple non-recursive JSON parser.
//
// The JSON specification is at http://www.ietf.org/rfc/rfc4627.txt
// The parser here will parse any valid JSON and return the correct
// corresponding Python object.  It allows through a superset of JSON, since
// it treats commas and colons as "whitespace", and doesn't care if
// brackets/braces are correctly paired.  It will raise a ValueError if the
// input is outside it's specs.
//
// Most of the work is parsing the primitives (null, false, true, numbers,
// strings).  It does 1 pass over the input stream.  It tries to be fast and
// small in code size, while not using more RAM than necessary.

typedef struct _eoslib_stream_t {
    mp_obj_t stream_obj;
    mp_uint_t (*read)(mp_obj_t obj, void *buf, mp_uint_t size, int *errcode);
    int errcode;
    byte cur;
} eoslib_stream_t;

#define S_EOF (0) // null is not allowed in json stream so is ok as EOF marker
#define S_END(s) ((s).cur == S_EOF)
#define S_CUR(s) ((s).cur)
#define S_NEXT(s) (eoslib_stream_next(&(s)))

STATIC mp_obj_t mod_eoslib_now(void) {
   return mp_obj_new_int(now_());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_eoslib_now_obj, mod_eoslib_now);

STATIC mp_obj_t mod_eoslib_read_message(void) {
   return read_message_();
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_eoslib_read_message_obj, mod_eoslib_read_message);

STATIC mp_obj_t mod_eoslib_require_scope(mp_obj_t obj) {
   uint64_t account = mp_obj_uint_get_checked(obj);
   require_scope_(account);
   return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_eoslib_require_scope_obj, mod_eoslib_require_scope);

STATIC mp_obj_t mod_eoslib_current_code(void) {
   return mp_obj_new_int_from_ull(current_code_());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_eoslib_current_code_obj, mod_eoslib_current_code);

STATIC mp_obj_t mod_eoslib_require_notice(mp_obj_t obj) {
   uint64_t account = mp_obj_uint_get_checked(obj);
   require_notice_(account);
   return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_eoslib_require_notice_obj, mod_eoslib_require_notice);

STATIC mp_obj_t mod_eoslib_string_to_uint64(mp_obj_t obj) {
   size_t len;
   const char *account = mp_obj_str_get_data(obj, &len);
   uint64_t n = string_to_uint64_(account);
   return mp_obj_new_int_from_ull(n);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_eoslib_string_to_uint64_obj, mod_eoslib_string_to_uint64);

STATIC mp_obj_t mod_eoslib_N(mp_obj_t obj) {
   size_t len;
   const char *account = mp_obj_str_get_data(obj, &len);
   uint64_t n = string_to_uint64_(account);
   return mp_obj_new_int_from_ull(n);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_eoslib_N_obj, mod_eoslib_N);

mp_uint_t mp_obj_uint_get_checked(mp_const_obj_t self_in);

STATIC mp_obj_t mod_eoslib_uint64_to_string(mp_obj_t obj) {
   uint64_t n = mp_obj_uint_get_checked(obj);
   return uint64_to_string_(n);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_eoslib_uint64_to_string_obj, mod_eoslib_uint64_to_string);

STATIC mp_obj_t mod_eoslib_pack(mp_obj_t obj) {
   size_t len = 0;
   const char *s = mp_obj_str_get_data(obj, &len);
   return pack_(s, len);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_eoslib_pack_obj, mod_eoslib_pack);

STATIC mp_obj_t mod_eoslib_unpack(mp_obj_t obj) {
   size_t len = 0;
   const char *s = mp_obj_str_get_data(obj, &len);
   return unpack_(s, len);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_eoslib_unpack_obj, mod_eoslib_unpack);

STATIC mp_obj_t mod_eoslib_store(size_t n_args, const mp_obj_t *args) {
   size_t keys_len = 0;
   size_t value_len = 0;
   uint64_t scope = mp_obj_uint_get_checked(args[0]);
   uint64_t table = mp_obj_uint_get_checked(args[1]);
   void* keys = (void *)mp_obj_str_get_data(args[2], &keys_len);
   int key_type = mp_obj_uint_get_checked(args[3]);
   void* value = (void *)mp_obj_str_get_data(args[4], &value_len);
   int ret = store_(scope, table, keys, keys_len, key_type, value, value_len);
   return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(mod_eoslib_store_obj, 5, mod_eoslib_store);


STATIC const mp_rom_map_elem_t mp_module_eoslib_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_eoslib) },
    { MP_ROM_QSTR(MP_QSTR_now), MP_ROM_PTR(&mod_eoslib_now_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_message), MP_ROM_PTR(&mod_eoslib_read_message_obj) },
    { MP_ROM_QSTR(MP_QSTR_require_scope), MP_ROM_PTR(&mod_eoslib_require_scope_obj) },
    { MP_ROM_QSTR(MP_QSTR_require_notice), MP_ROM_PTR(&mod_eoslib_require_notice_obj) },
    { MP_ROM_QSTR(MP_QSTR_current_code), MP_ROM_PTR(&mod_eoslib_current_code_obj) },
    { MP_ROM_QSTR(MP_QSTR_string_to_uint64), MP_ROM_PTR(&mod_eoslib_string_to_uint64_obj) },
    { MP_ROM_QSTR(MP_QSTR_N), MP_ROM_PTR(&mod_eoslib_N_obj) },
    { MP_ROM_QSTR(MP_QSTR_uint64_to_string), MP_ROM_PTR(&mod_eoslib_uint64_to_string_obj) },
    { MP_ROM_QSTR(MP_QSTR_pack), MP_ROM_PTR(&mod_eoslib_pack_obj) },
    { MP_ROM_QSTR(MP_QSTR_unpack), MP_ROM_PTR(&mod_eoslib_unpack_obj) },
    { MP_ROM_QSTR(MP_QSTR_store), MP_ROM_PTR(&mod_eoslib_store_obj) },

};

STATIC MP_DEFINE_CONST_DICT(mp_module_eoslib_globals, mp_module_eoslib_globals_table);

const mp_obj_module_t mp_module_eoslib = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_eoslib_globals,
};

#endif //MICROPY_PY_EOSLIB
