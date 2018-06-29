#include "vm_manager.hpp"

#include <eosio/chain/db_api.hpp>

#include <dlfcn.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include <eosio/chain/exceptions.hpp>

#include <appbase/application.hpp>
#include <appbase/platform.hpp>
#include <eosio/chain/contract_types.hpp>

#include <sys/time.h>
#include <time.h>
#include <unistd.h> // for sysconf

using namespace eosio;
using namespace eosio::chain;
namespace bio = boost::iostreams;


namespace eosio {
namespace chain {
   void register_vm_api(void* handle);
   int  wasm_to_wast( const uint8_t* data, size_t size, uint8_t* wast, size_t wast_size );
}
}

typedef struct vm_py_api* (*fn_get_py_vm_api)();
typedef struct vm_wasm_api* (*fn_get_wasm_vm_api)();
typedef uint64_t (*fn_wasm_call)(const char* act, uint64_t* args, int argc);

static vector<char> print_buffer;
static void print(const char * str, size_t len) {
   for (int i=0;i<len;i++) {
      if (str[i] == '\n') {
         string s(print_buffer.data(), print_buffer.size());
         print_buffer.clear();
         dlog(s);
         continue;
      }
      print_buffer.push_back(str[i]);
   }
}

uint64_t get_microseconds() {
   if (sysconf(_POSIX_THREAD_CPUTIME)){
      struct timespec tv;
      int err = clock_gettime(CLOCK_THREAD_CPUTIME_ID,&tv);
      if (err == 0) {
         return tv.tv_sec * 1000000LL + tv.tv_nsec / 1000LL;
      }
   }
   printf("+++++ERROR: something went wrong!\n");
   return 0;
}

template<size_t Limit>
struct read_limiter {
   using char_type = char;
   using category = bio::multichar_output_filter_tag;

   template<typename Sink>
   size_t write(Sink &sink, const char* s, size_t count)
   {
      EOS_ASSERT(_total + count <= Limit, tx_decompression_error, "Exceeded maximum decompressed transaction size");
      _total += count;
      return bio::write(sink, s, count);
   }

   size_t _total = 0;
};

void zlib_decompress_data(const bytes& data, bytes& out) {
   try {
      bio::filtering_ostream decomp;
      decomp.push(bio::zlib_decompressor());
      decomp.push(read_limiter<100*1024*1024>()); // limit to 10 megs decompressed for zip bomb protections
      decomp.push(bio::back_inserter(out));
      bio::write(decomp, data.data(), data.size());
      bio::close(decomp);
   } catch( fc::exception& er ) {
   } catch( ... ) {
   }
}

static uint64_t vm_names[] = {
#if defined(__APPLE__) && defined(__MACH__)
      N(vm.wasm.1),
      N(vm.py.1),
      N(vm.eth.1),
      N(vm.wasm.1),
      N(vm.wavm.1),
#elif defined(__linux__)
      N(vm.wasm.2),
      N(vm.py.2),
      N(vm.eth.2),
      N(vm.wavm.2),
#elif defined(_WIN64)
      N(vm.wasm.3),
      N(vm.py.3),
      N(vm.eth.3),
      N(vm.wavm.3),
#else
   #error Not Supported Platform
#endif
};

static const char* vm_libs_path[] = {
#ifdef DEBUG
   "../libs/libvm_wasm_binaryend" DYLIB_SUFFIX,
   "../libs/libvm_py-1d" DYLIB_SUFFIX,
   "../libs/libvm_ethd" DYLIB_SUFFIX,
   "../libs/libvm_wasm_wavmd" DYLIB_SUFFIX,
#else
   "../libs/libvm_wasm_binaryen" DYLIB_SUFFIX,
   "../libs/libvm_py-1" DYLIB_SUFFIX,
   "../libs/libvm_eth" DYLIB_SUFFIX,
   "../libs/libvm_wasm_wavm" DYLIB_SUFFIX,
#endif
};
vm_manager& vm_manager::get() {
   static vm_manager *mngr = nullptr;
   if (!mngr) {
      mngr = new vm_manager();
   }
   return *mngr;
}

bool vm_manager::init() {
   static bool init = false;
   if (init) {
      return true;
   }

   init = true;

   for (int i=0;i<sizeof(vm_libs_path)/sizeof(vm_libs_path[0]);i++) {
      if (load_vm(i, vm_names[i])) {
         continue;
      }
      load_vm_from_path(i, vm_libs_path[i]);
   }

   return true;
}

vm_manager::vm_manager() {
   init();
}

int vm_manager::load_vm_from_path(int vm_type, const char* vm_path) {
   uint64_t start = get_microseconds();

   void *handle = dlopen(vm_path, RTLD_LAZY | RTLD_LOCAL);
   if (handle == NULL) {
      return 0;
   }

   fn_vm_init vm_init = (fn_vm_init)dlsym(handle, "vm_init");
   if (vm_init == NULL) {
      return 0;
   }

   fn_vm_deinit vm_deinit = (fn_vm_deinit)dlsym(handle, "vm_deinit");
   if (vm_deinit == NULL) {
      return 0;
   }

   fn_setcode setcode = (fn_setcode)dlsym(handle, "setcode");
   if (setcode == NULL) {
      return 0;
   }

   fn_apply apply = (fn_apply)dlsym(handle, "apply");
   if (apply == NULL) {
      return 0;
   }

   vm_init();
   register_vm_api(handle);
   wlog("+++++++++++loading ${n1} cost: ${n2}", ("n1",vm_path)("n2", get_microseconds() - start));
   std::unique_ptr<vm_calls> calls = std::make_unique<vm_calls>();
   calls->version = 0;
   calls->handle = handle;
   calls->vm_init = vm_init;
   calls->vm_deinit = vm_deinit;
   calls->setcode = setcode;
   calls->apply = apply;
   wlog("loading ${n1} ${n2} ${n3}\n", ("n1", vm_path)("n2", (uint64_t)setcode)("n3", (uint64_t)apply));
   vm_map[vm_type] = std::move(calls);
   return 1;
}

int vm_manager::check_new_version(int vm_type, uint64_t vm_name) {
   uint64_t vm_store = N(vmstore);

   int itr = db_api::get().db_find_i64(vm_store, vm_store, vm_store, vm_name);
   if (itr < 0) {
      return 0;
   }

   size_t native_size = 0;
   const char* code = db_api::get().db_get_i64_exex(itr, &native_size);
   uint32_t type = *(uint32_t*)code;
   uint32_t version = *(uint32_t*)&code[4];
   uint32_t file_size = *(uint32_t*)&code[8];

   if (vm_type != type) {
      wlog("+++++++++type not match");
      return 0; //type not match
   }
   auto _itr = vm_map.find(vm_type);
   if (_itr == vm_map.end()) {
      return 1;
   }

   if (version > _itr->second->version) {
      return 1;
   }
   return 0;
}

int vm_manager::load_vm(int vm_type, uint64_t vm_name) {
   activatevm vm;
   uint64_t vm_store = N(vmstore);

   uint64_t start = get_microseconds();

   int itr = db_api::get().db_find_i64(N(eosio), N(eosio), N(eosio), vm_name);
   if (itr < 0) {
      return 0;
   }

   int size = db_api::get().db_get_i64(itr, (char*)&vm, sizeof(vm));

   if (size != sizeof(vm)) {
      return 0;
   }

   if (vm.type != vm_type) {
      return 0;
   }

   char _vm_name[128];
   snprintf(_vm_name, sizeof(_vm_name), "%s.%d", name(vm.vm_name).to_string().c_str(), (int)vm.version);
   uint64_t vm_name_with_version = NN(_vm_name);

   int _itr = db_api::get().db_find_i64(vm_store, vm_store, vm_store, vm_name_with_version);
   if (_itr < 0) {
      return 0;
   }

   size_t native_size = 0;
   const char* code = db_api::get().db_get_i64_exex(_itr, &native_size);
   uint32_t type = *(uint32_t*)code;
   uint32_t version = *(uint32_t*)&code[4];
   uint32_t file_size = *(uint32_t*)&code[8];
   uint32_t compressed_file_size = *(uint32_t*)&code[12];

   if (version != vm.version) {
      wlog("version not right!");
      return 0;
   }

   char vm_path[128];
   sprintf(vm_path, "%s.%d",name(vm_name).to_string().c_str(), version);

   wlog("loading vm ${n1}: ${n2}", ("n1", name(vm_name).to_string())("n2", vm_path));

   bytes compressed_data;
   compressed_data.reserve(compressed_file_size);

   int index = 1;
   while (true) {
      int itr = db_api::get().db_find_i64(vm_store, vm_store, vm_name, index);
      if (itr < 0) {
         break;
      }
      size_t native_size = 0;
      const char* code = db_api::get().db_get_i64_exex(itr, &native_size);
      compressed_data.insert(compressed_data.end(), code, code+native_size);
      index += 1;
   }

   bytes decompressed_data;
   zlib_decompress_data(compressed_data, decompressed_data);
   if (decompressed_data.size() != file_size) {
      return 0;
   }

   std::ofstream out(vm_path, std::ios::binary | std::ios::out);
   out.write(decompressed_data.data(), file_size);
   out.close();

   void *handle = dlopen(vm_path, RTLD_LAZY | RTLD_LOCAL);
   if (handle == NULL) {
      wlog("++++++++++++++dlopen ${n} failed", ("n", vm_path));
      return 0;
   }

   fn_vm_init vm_init = (fn_vm_init)dlsym(handle, "vm_init");
   if (vm_init == NULL) {
      return 0;
   }

   fn_vm_deinit vm_deinit = (fn_vm_deinit)dlsym(handle, "vm_deinit");
   if (vm_deinit == NULL) {
      return 0;
   }

   fn_setcode setcode = (fn_setcode)dlsym(handle, "setcode");
   if (setcode == NULL) {
      return 0;
   }

   fn_apply apply = (fn_apply)dlsym(handle, "apply");
   if (apply == NULL) {
      return 0;
   }

   vm_init();
   register_vm_api(handle);

   wlog("+++++++++++loading ${n1} cost: ${n2}", ("n1",vm_path)("n2", get_microseconds() - start));

   std::unique_ptr<vm_calls> calls = std::make_unique<vm_calls>();
   calls->handle = handle;
   calls->version = version;

   calls->vm_init = vm_init;
   calls->vm_deinit = vm_deinit;

   calls->setcode = setcode;
   calls->apply = apply;

   wlog("loading ${n1} ${n2} ${n3}\n", ("n1", vm_path)("n2", (uint64_t)setcode)("n3", (uint64_t)apply));
   auto __itr = vm_map.find(vm_type);
   if (__itr != vm_map.end()) {
      __itr->second->vm_deinit();
      dlclose(__itr->second->handle);
   }

   vm_map[vm_type] = std::move(calls);
   if (vm_type == 1) { //micropython
      get_py_vm_api(); //set printer
   }
   return 1;
}

int vm_manager::setcode(int type, uint64_t account) {
/*
   if (check_new_version(type, vm_names[type])) {
      load_vm(type, vm_names[type]);
   }
*/
   auto itr = vm_map.find(type);
   if (itr == vm_map.end()) {
      return -1;
   }
   return itr->second->setcode(account);
}

int vm_manager::apply(int type, uint64_t receiver, uint64_t account, uint64_t act) {
/*
   if (check_new_version(type, vm_names[type])) {
      load_vm(type, vm_names[type]);
   }
*/
   map<int, std::unique_ptr<vm_calls>>::iterator itr;

   if (type == 0) { //wasm
      // || receiver == N(eosio) || receiver == N(eosio.token)
      //appbase::app().has_option("hard-replay-blockchain")
      if (receiver == N(eosio) || receiver == N(eosio.token)) { //replay
         type = 3; //wavm
      }
   }

   itr = vm_map.find(type);

   if (itr == vm_map.end()) {
      return 0;
   }
   itr->second->apply(receiver, account, act);
   return 1;
}

struct vm_py_api* vm_manager::get_py_vm_api() {
   auto itr = vm_map.find(1);
   if (itr == vm_map.end()) {
      return nullptr;
   }

   fn_get_py_vm_api _get_py_vm_api = (fn_get_py_vm_api)dlsym(itr->second->handle, "get_py_vm_api");
   if (_get_py_vm_api == nullptr) {
      return nullptr;
   }

   struct vm_py_api* api = _get_py_vm_api();
   api->set_printer(print);
   return api;
}

void *vm_manager::get_eth_vm_api() {
   return nullptr;
}

struct vm_wasm_api* vm_manager::get_wasm_vm_api() {
   auto itr = vm_map.find(0);
   if (itr == vm_map.end()) {
      return nullptr;
   }

   fn_get_wasm_vm_api _get_wasm_vm_api = (fn_get_wasm_vm_api)dlsym(itr->second->handle, "get_wasm_vm_api");
   if (_get_wasm_vm_api == nullptr) {
      return nullptr;
   }

   struct vm_wasm_api* api = _get_wasm_vm_api();
   return api;
}

uint64_t vm_manager::wasm_call(const string& func, vector<uint64_t> args) {
   auto itr = vm_map.find(0);
   if (itr == vm_map.end()) {
      return -1;
   }
   uint64_t _args[args.size()];
   for (int i=0;i<args.size();i++) {
      _args[i] = args[i];
   }
   fn_wasm_call _wasm_call = (fn_wasm_call)dlsym(itr->second->handle, "call");
   if (_wasm_call == nullptr) {
      return -1;
   }

   return _wasm_call(func.c_str(), _args, args.size());
}

namespace eosio { namespace chain {

   std::vector<uint8_t> _wast_to_wasm( const std::string& wast ) {
      std::vector<uint8_t> v(wast.size()*2);
      struct vm_wasm_api* api = vm_manager::get().get_wasm_vm_api();
      int size = api->wast_to_wasm( (uint8_t*)wast.c_str(), wast.size(), v.data(), v.size());
      return std::vector<uint8_t>(v.data(), v.data()+size);
   }

   std::string  _wasm_to_wast( const uint8_t* data, uint64_t size ) {
      struct vm_wasm_api* api = vm_manager::get().get_wasm_vm_api();
      int wast_size = api->wasm_to_wast( (uint8_t*)data, size, nullptr, 0);

      std::vector<uint8_t> v(wast_size);
      wast_size = api->wasm_to_wast( (uint8_t*)data, size, v.data(), v.size());
      return std::string((char*)v.data(), wast_size);
   }

}
}
