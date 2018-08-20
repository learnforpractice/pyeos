#define JULIA_ENABLE_THREADING
#include <julia.h>
#include "julia_internal.h"
#include <builtin_proto.h>
#include <eosiolib_native/vm_api.h>

void add_builtin_func(const char *name, jl_fptr_args_t fptr);

JL_CALLABLE(jl_f__read_action)
{
   printf("+++jl_f__read_action\n");
   JL_NARGSV(read_action, 0);
    jl_value_t *a = args[0], *b = args[1];

    auto len = action_data_size();
    void *msg = malloc(len);
    read_action_data(msg, len );
    jl_value_t *ret = jl_pchar_to_string((char*)msg, len);
    free(msg);
    return ret;
}

void vm_add_builtin_funcs(void) {
   printf("-----------------vm_add_builtin_funcs\n");
//   add_builtin_func("read_action", jl_f__read_action);
}
