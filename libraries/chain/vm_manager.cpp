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
   char* dl_path;
#ifdef DEBUG
   dl_path = "../libraries/micropython/libmicropython-1d.dylib";
//   sprintf(dl_path, "../libraries/micropython/libmicropython-1d.dylib", counter);
#else
   dl_path = "../libraries/micropython/libmicropython-1.dylib";
//   sprintf(dl_path, "../libraries/micropython/libmicropython-1.dylib", counter);
#endif
   void *handle = dlopen(dl_path, RTLD_LAZY | RTLD_LOCAL);
   fn_setcode setcode = (fn_setcode)dlsym(handle, "setcode");
   fn_apply apply = (fn_apply)dlsym(handle, "apply");

   std::unique_ptr<vm_calls> calls = std::make_unique<vm_calls>();
   calls->handle = handle;
   calls->setcode = setcode;
   calls->apply = apply;
   vm_map[1] = std::move(calls);
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

