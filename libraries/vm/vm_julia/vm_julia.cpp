#include <eosiolib_native/vm_api.h>

#define JULIA_ENABLE_THREADING
#include <julia.h>

#include <map>
using namespace std;

static map<uint64_t, jl_module_t*> module_map;

const char* get_code( uint64_t receiver, size_t* size ) {
   return get_vm_api()->get_code( receiver, size );
}

void vm_init(struct vm_api* api) {
   printf("vm_julia: init\n");
   vm_register_api(api);
}

void vm_deinit() {
   printf("vm_julia: deinit\n");
//   jl_atexit_hook(0);
}

static bool _init = false;

int vm_setcode(uint64_t account) {
   printf("+++++vm_julia: setcode\n");
   if (!_init) {
      jl_init();
      _init = true;
   }

   size_t size;
   jl_module_t * mod;
   const char* code = get_code(account, &size);
   auto ret = jl_eval_string(code);

   if (jl_exception_occurred()) {
      eosio_assert(0, jl_typeof_str(jl_exception_occurred()));
      return 0;
   }

   eosio_assert(ret != nullptr, "load module failure");
   eosio_assert(jl_typeis(ret, jl_module_type), "code is not a module");

   mod = (jl_module_t *)ret;
   jl_function_t *func = jl_get_function(mod, "apply");
   eosio_assert(func != nullptr, "apply not found");
   module_map[account] = mod;
   return 0;
}

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   printf("+++++vm_julia: apply\n");
//   jl_eval_string("print(sqrt(2.0))");
   if (!_init) {
      jl_init();
      _init = true;
   }

   jl_module_t * mod;
   auto it = module_map.find(receiver);
   if (it == module_map.end()) {
      size_t size;
      const char* code = get_code(receiver, &size);
      auto ret = jl_eval_string(code);
      if (jl_exception_occurred()) {
         eosio_assert(0, jl_typeof_str(jl_exception_occurred()));
         return 0;
      }
      eosio_assert(ret != nullptr, "load module failure");
      eosio_assert(jl_typeis(ret, jl_module_type), "code is not a module");
      mod = (jl_module_t *)ret;
      module_map[receiver] = mod;
   } else {
      mod = it->second;
   }

   jl_function_t *func = jl_get_function(mod, "apply");
   eosio_assert(func != nullptr, "apply not found");

   jl_value_t *arg1 = jl_box_uint64(receiver);
   jl_value_t *arg2 = jl_box_uint64(account);
   jl_value_t *arg3 = jl_box_uint64(act);
   jl_value_t *ret = jl_call3(func, arg1, arg2, arg3);
   if (jl_exception_occurred()) {
      printf("%s \n", jl_typeof_str(jl_exception_occurred()));
      return 0;
   }
   if (ret == NULL) {
      printf("something went wrong...\n");
      return 0;
   }
   int _ret = 0;
   if (jl_typeis(ret, jl_int64_type)) {
      _ret = jl_unbox_int64(ret);
   }
   return _ret;
}

int vm_call(uint64_t account, uint64_t func) {
   printf("+++++vm_julia: call\n");
   return 0;
}

