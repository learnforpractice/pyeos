#include "vm_manager.hpp"
#include "micropython/db_api.hpp"
#include <dlfcn.h>

vm_manager& vm_manager::get() {
   static vm_manager *mngr = nullptr;
   if (!mngr) {
      mngr = new vm_manager();
   }
   return *mngr;
}

vm_manager::vm_manager() {
   const char* vm_libs_path[] = {
#ifdef DEBUG
   "../libraries/vm_wasm/libvm_wasmd.dylib",
   "../libraries/vm_py/libvm_py-1d.dylib",
   "../libraries/vm_eth/libvm_ethd.dylib",
#else
   "../libraries/vm_wasm/libvm_wasm.dylib",
   "../libraries/vm_py/libvm_py.dylib",
   "../libraries/vm_eth/libvm_eth.dylib",
#endif
   };

   int vm_types[] = {0, 1, 2};
   for (int i=0;i<3;i++) {
      void *handle = dlopen(vm_libs_path[i], RTLD_LAZY | RTLD_LOCAL);
      if (handle == NULL) {
         continue;
      }

      fn_setcode setcode = (fn_setcode)dlsym(handle, "setcode");
      if (setcode == NULL) {
         continue;
      }

      fn_apply apply = (fn_apply)dlsym(handle, "apply");
      if (apply == NULL) {
         continue;
      }

      std::unique_ptr<vm_calls> calls = std::make_unique<vm_calls>();
      calls->handle = handle;
      calls->setcode = setcode;
      calls->apply = apply;
      printf("loading %s %p %p\n", vm_libs_path[i], setcode, apply);
      vm_map[vm_types[i]] = std::move(calls);
   }
}

int vm_manager::setcode(int type, uint64_t account) {
   auto itr = vm_map.find(type);
   if (itr == vm_map.end()) {
      return -1;
   }
   return itr->second->setcode(account);
}

int vm_manager::apply(int type, uint64_t receiver, uint64_t account, uint64_t act) {
   auto itr = vm_map.find(type);
   if (itr == vm_map.end()) {
      return -1;
   }
   return itr->second->apply(receiver, account, act);
}

