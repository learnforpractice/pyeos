#include <eosiolib_native/vm_api.h>
#include <luasandbox.h>
#include <map>

typedef int (*fn_check_time)(void);

extern "C" lsb_err_value lsb_init_ex(lsb_lua_sandbox *lsb, const char *state_file, const char* str_code);
extern "C" void luaV_set_check_time_fn(fn_check_time fn);
void lsb_register_vm_api(lsb_lua_sandbox *lsb);

int check_time() {
   try {
      get_vm_api()->checktime();
   } catch (...) {
      return 0;
   }
   return 1;
}

static std::map<uint64_t, lsb_lua_sandbox *> account_map;

static char print_out[2048] = { 0 };

void print(void *context, const char *component, int level, const char *fmt, ...)
{
   (void)context;
   va_list args;
   int n = snprintf(print_out, sizeof print_out, "%d %s ", level,
                   component ? component : "unnamed");
   va_start(args, fmt);
   n = vsnprintf(print_out + n, sizeof print_out - n, fmt, args);
   va_end(args);
   vmdlog(print_out);
}

static lsb_logger printer = { .context = NULL, .cb = print };

int vm_run_lua_script(const char* cfg, const char* script) {
   luaV_set_check_time_fn(NULL);

   lsb_lua_sandbox *lsb = lsb_create(NULL, "null.lua", cfg, &printer);
   if (!lsb) {
      return 0;
   }

   lsb_err_value ret = lsb_init_ex(lsb, NULL, script);
   luaV_set_check_time_fn(check_time);

   if (ret) {

      const char* error = lsb_get_error(lsb);
      if (error) {
         vmdlog("script error: %s \n", error);
      }

      lsb_terminate(lsb, NULL);
      lsb_destroy(lsb);
      return 0;
   }
   static const char *func_name = "apply";
   lua_State *lua = lsb_get_lua(lsb);

   lsb_terminate(lsb, NULL);
   lsb_destroy(lsb);
   return 1;
}


/*
static const char *cfg = "memory_limit = 65765\n"
    "instruction_limit = 1000\n"
    "output_limit = 1024\n"
    "array = {'foo', 99}\n"
    "hash  = {foo = 'bar', hash1 = {subfoo = 'subbar'}}\n"
*/
static const char *cfg = "memory_limit = 1024*1024\n"
      "disabled_modules = {io = 0}\n"
    "output_limit = 1024\n"
    "log_level = 7\n";

lsb_lua_sandbox *load_account(uint64_t account) {
   size_t size = 0;
   const char* str_code = get_code(account, &size);
   if (size <= 0) {
      return NULL;
   }

   lsb_lua_sandbox *lsb = lsb_create(NULL, "null.lua", cfg, &printer);
   if (!lsb) {
      return NULL;
   }

   lsb_register_vm_api(lsb);

   lsb_err_value ret = lsb_init_ex(lsb, NULL, str_code);
   if (ret) {
      lsb_terminate(lsb, NULL);
      lsb_destroy(lsb);
      const char* error = lsb_get_error(lsb);
      if (error) {
         eosio_assert(0, error);
      } else {
         eosio_assert(0, "unknown error!");
      }
      return NULL;
   }

   static const char *func_name = "apply";
   lua_State *lua = lsb_get_lua(lsb);
   if (!lua) {
      lsb_terminate(lsb, NULL);
      lsb_destroy(lsb);
      const char* error = lsb_get_error(lsb);
      if (error) {
         eosio_assert(0, error);
      } else {
         eosio_assert(0, "unknown error!");
      }
      return NULL;
   }

   if (lsb_pcall_setup(lsb, func_name)) {
      lsb_terminate(lsb, NULL);
      lsb_destroy(lsb);
      return NULL;
   }

   account_map[account] = lsb;
   return lsb;
}

int _apply(lsb_lua_sandbox *lsb, uint64_t receiver, uint64_t account, uint64_t act)
{
   static const char *func_name = "apply";
   lua_State *lua = lsb_get_lua(lsb);
   if (!lua) return 0;

   if (lsb_pcall_setup(lsb, func_name)) return 1;

   int top = lua_gettop(lua);

   lua_pushnumber(lua, receiver);
   lua_pushnumber(lua, account);
   lua_pushnumber(lua, act);

   if (lua_pcall(lua, 3, 1, 0) != 0) {
      char err[LSB_ERROR_SIZE];
      const char *em = lua_tostring(lua, -1);
      int len = snprintf(err, LSB_ERROR_SIZE, "%s() %s", func_name,
                       em ? em : LSB_NIL_ERROR);
      if (len >= LSB_ERROR_SIZE || len < 0) {
      err[LSB_ERROR_SIZE - 1] = 0;
      }
      lsb_set_error(lsb, err);
      //    lsb_terminate(lsb, err);
      return 0;
   }
#if 0
   int nresults = lua_gettop(lua) - top;
//   printf("++++++++++++nresults: %d \n", nresults);

   if (!lua_isnumber(lua, -1)) {
      char err[LSB_ERROR_SIZE];
      int len = snprintf(err, LSB_ERROR_SIZE,
                       "%s() must return a number", func_name);
      if (len >= LSB_ERROR_SIZE || len < 0) {
         err[LSB_ERROR_SIZE - 1] = 0;
      }
      lsb_set_error(lsb, err);
      //    lsb_terminate(lsb, err);
      return 0;
   }

   int status = (int)lua_tointeger(lua, -1);
#endif
   lua_pop(lua, 1);

   lsb_pcall_teardown(lsb);

   return 1;
}

void vm_init(struct vm_api* api) {
   int ok;
   printf("vm_lua: init\n");
   api->vm_run_lua_script = vm_run_lua_script;
   vm_register_api(api);
   luaV_set_check_time_fn(check_time);
//   lua_State *L;
//   L=lua_open();
//   ok=dostring(L,"print('hello,lua world')");
}

void vm_deinit() {
   printf("vm_lua: deinit\n");
}

int vm_setcode(uint64_t account) {
   printf("+++++vm_lua: setcode\n");
   auto itr = account_map.find(account);
   if (itr != account_map.end()) {
      lsb_lua_sandbox *lsb = itr->second;
      lsb_terminate(lsb, NULL);
      lsb_destroy(lsb);
   }
   load_account(account);
   return 0;
}

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
//   printf("+++++vm_lua: apply\n");
   auto itr = account_map.find(receiver);
   lsb_lua_sandbox *lsb;
   if (itr == account_map.end()) {
      lsb = load_account(receiver);
   } else {
      lsb = itr->second;
   }

   if (!lsb) {
      return 0;
   }

   int ret = _apply(lsb, receiver, account, act);
   if (ret == 0) {
      const char* error = lsb_get_error(lsb);
      if (error) {
         eosio_assert(0, error);
      } else {
         eosio_assert(0, "unknown error!");
      }
   }
   return 1;
}

int vm_call(uint64_t account, uint64_t func) {
   printf("+++++vm_lua: call\n");
   return 0;
}
