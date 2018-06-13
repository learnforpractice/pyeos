#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <map>
#include <string>
#include <vector>
#include <memory>

#include <vm_py_api.h>

using namespace std;

typedef int (*fn_setcode)(uint64_t account);
typedef int (*fn_apply)(uint64_t receiver, uint64_t account, uint64_t act);
typedef void (*fn_init_vm)();

struct vm_calls {
   uint32_t version;
   void* handle;
   fn_setcode setcode;
   fn_apply apply;
};

class vm_manager
{
public:
   static vm_manager& get();
   int setcode(int type, uint64_t account);
   int apply(int type, uint64_t receiver, uint64_t account, uint64_t act);
   int check_new_version(int vm_type, uint64_t vm_name);
   int load_vm_default(int vm_type, const char* vm_path);
   int load_vm(int vm_type, uint64_t vm_name);
   bool init();

   void *get_wasm_vm_api();
   struct vm_py_api* get_py_vm_api();
   void *get_eth_vm_api();

private:
   vm_manager();
   map<int, std::unique_ptr<vm_calls>> vm_map;
};

