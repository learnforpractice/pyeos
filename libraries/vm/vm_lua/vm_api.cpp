#include <stdlib.h>
#include <string.h>

extern "C" {
#include <luasandbox.h>
#include <luasandbox/lauxlib.h>
}

#include <eosiolib_native/vm_api.h>

#include <vector>
using namespace std;

static int is_account_ (lua_State *L) {
   uint64_t account = luaL_checknumber(L, 1);
   int ret = is_account(account);
   lua_pushboolean(L, ret);
   return 1;
}

static int s2n_ (lua_State *L) {
   size_t size;
   const char* str_n = luaL_checklstring(L, 1, &size);
   uint64_t n = get_vm_api()->string_to_uint64(str_n);
   lua_pushnumber(L, n);
   return 1;
}

static int n2s_(lua_State *L) {
   char buf[13];
   uint64_t n = luaL_checknumber(L, 1);
   int size = get_vm_api()->uint64_to_string(n, buf, sizeof(buf));
   lua_pushlstring(L, buf, size);
   return 1;
}

static int action_data_size_ (lua_State *L) {
   size_t size = action_data_size();
   lua_pushinteger(L, size);
   return 1;
}

static int read_action_data_ (lua_State *L) {
   size_t size = action_data_size();
   printf("++++++read_action_data: %d \n", size);

   if (size <= 0) {
      lua_pushnil(L);
      return 1;
   }
   vector<char> buffer(size);
   read_action_data(buffer.data(), size);
   lua_pushlstring(L, buffer.data(), size);
   return 1;
}

static int require_recipient_ (lua_State *L) {
   uint64_t account = luaL_checknumber(L, 1);
   require_recipient(account);
   return 0;
}

static int require_auth_ (lua_State *L) {
   uint64_t account = luaL_checknumber(L, 1);
   require_auth(account);
   return 0;
}

static int db_store_i64_ (lua_State *L) {
   uint64_t scope = luaL_checknumber(L, 1);
   uint64_t table = luaL_checknumber(L, 2);
   uint64_t payer = luaL_checknumber(L, 3);
   uint64_t id = luaL_checknumber(L, 4);
   size_t size = 0;
   const char *data = luaL_checklstring(L, 5, &size);
   int ret = db_store_i64(scope, table, payer, id, data, size);
   lua_pushinteger(L, ret);
   return 0;
}

static int db_update_i64_ (lua_State *L) {
   int itr = luaL_checkinteger(L, 1);
   uint64_t payer = luaL_checknumber(L, 2);
   size_t size = 0;
   const char *data = luaL_checklstring(L, 3, &size);
   db_update_i64(itr, payer, data, size);
   return 0;
}

static int db_remove_i64_(lua_State *L) {
   int itr = luaL_checkinteger(L, 1);
   db_remove_i64(itr);
   return 0;
}

static int db_get_i64_(lua_State *L) {
   int itr = luaL_checkinteger(L, 1);
   size_t size = db_get_i64( itr, NULL, 0 );
   if (size <= 0) {
      lua_pushnil(L);
      return 1;
   }
   vector<char> data(size);
   db_get_i64(itr, data.data(), size);
   lua_pushlstring(L, data.data(), size);
   return 1;
}

static int db_next_i64_(lua_State *L) {
   int itr = luaL_checkinteger(L, 1);
   uint64_t primary = 0;
   int itr_next = db_next_i64(itr, &primary);
   lua_pushinteger(L, itr_next);
   lua_pushnumber(L, primary);
   return 2;
}

static int db_previous_i64_(lua_State *L) {
   int itr = luaL_checkinteger(L, 1);
   uint64_t primary = 0;
   int itr_next = db_previous_i64(itr, &primary);
   lua_pushinteger(L, itr_next);
   lua_pushnumber(L, primary);
   return 2;
}

static int db_find_i64_(lua_State *L) {
   uint64_t code = luaL_checknumber(L, 1);
   uint64_t scope = luaL_checknumber(L, 2);
   uint64_t table = luaL_checknumber(L, 3);
   uint64_t id = luaL_checknumber(L, 4);

   int itr = db_find_i64(code, scope, table, id);
   lua_pushinteger(L, itr);
   return 1;
}

static int db_lowerbound_i64_(lua_State *L) {
   uint64_t code = luaL_checknumber(L, 1);
   uint64_t scope = luaL_checknumber(L, 2);
   uint64_t table = luaL_checknumber(L, 3);
   uint64_t id = luaL_checknumber(L, 4);

   int itr = db_lowerbound_i64(code, scope, table, id);
   lua_pushinteger(L, itr);
   return 1;
}

static int db_upperbound_i64_(lua_State *L) {
   uint64_t code = luaL_checknumber(L, 1);
   uint64_t scope = luaL_checknumber(L, 2);
   uint64_t table = luaL_checknumber(L, 3);
   uint64_t id = luaL_checknumber(L, 4);

   int itr = db_upperbound_i64(code, scope, table, id);
   lua_pushinteger(L, itr);
   return 1;
}

static int db_end_i64_(lua_State *L) {
   uint64_t code = luaL_checknumber(L, 1);
   uint64_t scope = luaL_checknumber(L, 2);
   uint64_t table = luaL_checknumber(L, 3);

   int itr = db_end_i64(code, scope, table);
   lua_pushinteger(L, itr);
   return 1;
}

static int rshift_(lua_State *L) {
   uint64_t n = luaL_checknumber(L, 1);
   uint64_t by = luaL_checknumber(L, 2);
   n = n>>by;
   lua_pushinteger(L, n);
   return 1;
}

static int lshift_(lua_State *L) {
   uint64_t n = luaL_checknumber(L, 1);
   uint64_t by = luaL_checknumber(L, 2);
   n = n<<by;
   lua_pushinteger(L, n);
   return 1;
}

void lsb_register_vm_api(lsb_lua_sandbox *lsb) {
   lsb_add_function(lsb, is_account_,                 "is_account");
   lsb_add_function(lsb, read_action_data_,           "read_action_data");
   lsb_add_function(lsb, action_data_size_,           "action_data_size");
   lsb_add_function(lsb, require_recipient_,          "require_recipient");
   lsb_add_function(lsb, require_auth_,               "require_auth");
   lsb_add_function(lsb, db_store_i64_,               "db_store_i64");
   lsb_add_function(lsb, db_update_i64_,              "db_update_i64");
   lsb_add_function(lsb, db_remove_i64_,              "db_remove_i64");
   lsb_add_function(lsb, db_get_i64_,                 "db_get_i64");
   lsb_add_function(lsb, db_next_i64_,                "db_next_i64");
   lsb_add_function(lsb, db_previous_i64_,            "db_previous_i64");
   lsb_add_function(lsb, db_find_i64_,                "db_find_i64");
   lsb_add_function(lsb, db_lowerbound_i64_,          "db_lowerbound_i64");
   lsb_add_function(lsb, db_upperbound_i64_,          "db_upperbound_i64");
   lsb_add_function(lsb, db_end_i64_,                 "db_end_i64");
   lsb_add_function(lsb, s2n_,                        "s2n");
   lsb_add_function(lsb, s2n_,                        "N");
   lsb_add_function(lsb, n2s_,                        "n2s");
   lsb_add_function(lsb, rshift_,                     "rshift");
   lsb_add_function(lsb, lshift_,                     "lshift");
}


