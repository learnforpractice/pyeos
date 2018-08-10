#include "vm_manager.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/thread/thread.hpp>

#include <eosio/chain/exceptions.hpp>

#include <appbase/application.hpp>
#include <appbase/platform.hpp>
#include <eosio/chain/contract_types.hpp>

#include <sys/time.h>
#include <time.h>
#include <unistd.h> // for sysconf

#include <thread>
#include <mutex>
#include <dlfcn.h>
#include <eosiolib/system.h>

using namespace eosio;
using namespace eosio::chain;
namespace bio = boost::iostreams;


static const int VM_TYPE_BINARYEN = 0;
static const int VM_TYPE_PY = 1;
static const int VM_TYPE_ETH = 2;
static const int VM_TYPE_WAVM = 3;
static const int VM_TYPE_IPC = 4;
static const int VM_TYPE_NATIVE = 5;
static const int VM_TYPE_CPYTHON = 6;
static const int VM_TYPE_CPYTHON_SS = 7;


namespace eosio {
namespace chain {
   int  wasm_to_wast( const uint8_t* data, size_t size, uint8_t* wast, size_t wast_size );
}
}

#define WAVM_VM_START_INDEX (0x10000)


typedef void (*fn_on_boost_account)(void* v, uint64_t account, uint64_t expiration);
void visit_boost_account(fn_on_boost_account fn, void* param);

typedef struct vm_py_api* (*fn_get_py_vm_api)();
typedef struct vm_wasm_api* (*fn_get_wasm_vm_api)();
typedef uint64_t (*fn_wasm_call)(const char* act, uint64_t* args, int argc);
bool is_boost_account(uint64_t account, bool& expired);

//eosio.prods.cpp
extern "C" bool is_jit_account_activated(uint64_t account);


extern "C" bool is_server_mode();

void _on_boost_account(void* v, uint64_t account, uint64_t expiration) {
   vm_manager* mngr = (vm_manager*)v;
   if (expiration < fc::time_point::now().time_since_epoch().count()) {
      return;
   }
   mngr->on_boost_account(account);
}

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
      N(vm.wavm.1),
      N(vm.ipc.1),
#elif defined(__linux__)
      N(vm.wasm.2),
      N(vm.py.2),
      N(vm.eth.2),
      N(vm.wavm.2),
      N(vm.ipc.2),
#elif defined(_WIN64)
      N(vm.wasm.3),
      N(vm.py.3),
      N(vm.eth.3),
      N(vm.wavm.3),
      N(vm.ipc.3),
#else
   #error Not Supported Platform
#endif
};

static const char* vm_libs_path[] = {
   "../libs/libvm_wasm_binaryen" DYLIB_SUFFIX,
   "../libs/libvm_py-1" DYLIB_SUFFIX,
   "../libs/libvm_eth" DYLIB_SUFFIX,
   "../libs/libvm_wasm_wavm-0" DYLIB_SUFFIX,
//   "../libs/libipc_server" DYLIB_SUFFIX
};

static const char *ipc_server_lib = "../libs/libipc_server" DYLIB_SUFFIX;
static const char *vm_native_lib = "../libs/libvm_native" DYLIB_SUFFIX;
static const char *vm_cpython_lib = "../libs/libvm_cpython" DYLIB_SUFFIX;
static const char *vm_cpython_ss_lib = "../libs/libvm_cpython_ss" DYLIB_SUFFIX;

vm_manager& vm_manager::get() {
   static vm_manager *mngr = nullptr;
   if (!mngr) {
      mngr = new vm_manager();
   }
   return *mngr;
}

class time_counter {
public:
   time_counter(uint64_t _account) {
      account = _account;
      start = get_microseconds();
   }
   ~time_counter() {
      wlog("loading ${n1} cost ${n2}", ("n1", name(account))("n2", get_microseconds()-start));
   }
private:
   uint64_t account;
   uint64_t start;
};

bool vm_manager::init(struct vm_api* api) {
   static bool init = false;
   if (init) {
      return true;
   }

   init = true;

   this->api = api;

   for (int i=0;i<3;i++) {
      if (load_vm_from_ram(i, vm_names[i])) {
         continue;
      }
      load_vm_from_path(i, vm_libs_path[i]);
   }

   load_vm_wavm();

   if (get_vm_api()->has_option("use-ipc")) {
      if (this->api->run_mode() == 0) {//server
         load_vm_from_path(VM_TYPE_IPC, ipc_server_lib);
      }
   }

   load_vm_from_path(VM_TYPE_NATIVE, vm_native_lib);

   load_vm_from_path(VM_TYPE_CPYTHON, vm_cpython_lib);
   load_vm_from_path(VM_TYPE_CPYTHON_SS, vm_cpython_ss_lib);

   return true;
}

int vm_manager::load_vm_wavm() {
   int default_wavm_index = 3;
   if (!load_vm_from_ram(default_wavm_index, vm_names[default_wavm_index])) {
      load_vm_from_path(default_wavm_index, vm_libs_path[default_wavm_index]);
   }

   char _path[128];
   const char* _format = "../libs/libvm_wasm_wavm-%d" DYLIB_SUFFIX;

   for (int i=1;i<=10;i++) {
      snprintf(_path, sizeof(_path), _format, i);
      load_vm_from_path(WAVM_VM_START_INDEX|i, _path);
   }

   visit_boost_account(_on_boost_account, this);

   auto itr = vm_map.find(3);
   if (itr != vm_map.end()) {
      for (uint64_t account : {N(eosio.token), N(eosio)}) {
         if (db_api::get().is_account(account) && db_api::get().get_code(account).size() > 0) {
            if (db_api::get().get_code_type(account) == 0) {
               auto t = time_counter(account);
               itr->second->preload(account);
               std::unique_ptr<vm_calls> calls = std::make_unique<vm_calls>();
               *calls = *itr->second;
               preload_account_map[account] = std::move(calls);
            }
         }
      }
   }

   if (boost_accounts.size() > 0) {
      boost::thread_group g;

      int cpu_num = std::thread::hardware_concurrency();
      if (cpu_num <= 0) {
         cpu_num = 1;
      }

      wlog("preloading code in ${n} threads", ("n", cpu_num));

      for (int i=1;i<=cpu_num;i++) {//TODO: 10 --> number of CPU cores
         auto itr = vm_map.find(WAVM_VM_START_INDEX|i);
         if (itr == vm_map.end()) {
            continue;
         }
         if (!itr->second->preload) {
            continue;
         }

         const auto _calls = itr->second.get();
         g.create_thread( boost::bind( &vm_manager::preload_accounts,this, _calls ) );
      }
      g.join_all();
   }
   return 1;
}

void vm_manager::preload_accounts(vm_calls* _calls) {
   static std::mutex m1, m2;
   while(true) {
      uint64_t account;
      {
         std::lock_guard<std::mutex> lock(m1);
         if (boost_accounts.empty()) {
            break;
         }
         account = boost_accounts.back();
         boost_accounts.pop_back();
      }
      {
         if (account == N(eosio) || account == N(eosio.token)) {
            continue;//already loaded in vm type 3
         }
         {
            auto t = time_counter(account);
            _calls->preload(account);
         }

         std::unique_ptr<vm_calls> calls = std::make_unique<vm_calls>();
         *calls = *_calls;
         {
            std::lock_guard<std::mutex> lock(m2);
            preload_account_map[account] = std::move(calls);
         }
      }
   }
}

void vm_manager::unload_account(uint64_t account) {
   auto itr = preload_account_map.find(account);
   if (itr != preload_account_map.end()) {
      itr->second->unload(account);
      preload_account_map.erase(itr);
   }

   auto _itr = vm_map.find(3);
   if (_itr != vm_map.end()) {
      _itr->second->unload(account);
   }
}

void vm_manager::on_boost_account(uint64_t account) {
   boost_accounts.push_back(account);
}

vm_manager::vm_manager() {
//   init();
   this->api = nullptr;
   trusted_accounts[N(eosio)] = N(eosio);
   trusted_accounts[N(eosio.token)] = N(eosio.token);
}

int vm_manager::load_vm_from_path(int vm_type, const char* vm_path) {
   uint64_t start = get_microseconds();

   void *handle = dlopen(vm_path, RTLD_LAZY | RTLD_LOCAL);
   if (handle == NULL) {
      elog("load ${n1} failed: ${n2}", ("n1", dlerror())("n2",vm_path));
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

   fn_setcode setcode = (fn_setcode)dlsym(handle, "vm_setcode");
   if (setcode == NULL) {
      return 0;
   }

   fn_apply apply = (fn_apply)dlsym(handle, "vm_apply");
   if (apply == NULL) {
      return 0;
   }

   fn_call _call = (fn_call)dlsym(handle, "vm_call");
   if (_call == NULL) {
      return 0;
   }

   fn_preload preload = (fn_preload)dlsym(handle, "vm_preload");
   /*
   if (preload == NULL) {
      return 0;
   }
   */
   fn_unload unload = (fn_unload)dlsym(handle, "vm_unload");

   auto __itr = vm_map.find(vm_type);
   if (__itr != vm_map.end()) {
      __itr->second->vm_deinit();
      dlclose(__itr->second->handle);
   }

   wlog("+++++++++++loading ${n1} cost: ${n2}", ("n1",vm_path)("n2", get_microseconds() - start));
   vm_init(this->api);

   std::unique_ptr<vm_calls> calls = std::make_unique<vm_calls>();
   calls->version = 0;
   calls->handle = handle;
   calls->vm_init = vm_init;
   calls->vm_deinit = vm_deinit;
   calls->setcode = setcode;
   calls->apply = apply;
   calls->call = _call;
   calls->preload = preload;
   calls->unload = unload;

   vm_map[vm_type] = std::move(calls);
   if (vm_type == 1) { //micropython
      get_py_vm_api(); //set printer
   }
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

int vm_manager::load_vm_from_ram(int vm_type, uint64_t vm_name) {
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

   return load_vm_from_path(vm_type, vm_path);
}

int vm_manager::load_vm(int vm_type) {
   if (vm_type == 3) { //wavm
      return load_vm_wavm();
   }
   return load_vm_from_path(vm_type, vm_libs_path[vm_type]);
}

bool vm_manager::is_trusted_account(uint64_t account) {
   if (trusted_accounts.find(account) != trusted_accounts.end()) {
      return true;
   }
   return false;

}

struct vm_api* vm_manager::get_vm_api() {
   return this->api;
}

int vm_manager::setcode(int type, uint64_t account) {
/*
   if (check_new_version(type, vm_names[type])) {
      load_vm_from_ram(type, vm_names[type]);
   }
*/
   if (type == VM_TYPE_CPYTHON) {
      if (this->api->is_privileged(account)) {
      } else {
         EOS_ASSERT(this->api->has_option("debug"), fc::assert_exception, "set code no allowed");
      }
   }
   if (this->api->run_mode() == 0) {
      if (is_trusted_account(account)) {
      } else {
         if (vm_map.find(VM_TYPE_IPC) != vm_map.end()) {
            type = VM_TYPE_IPC;
         }
      }
   }

   if (type == 0) {
      if (account == N(eosio) || account == N(eosio.token)) {
         //type = 3;
      }
   }

   auto itr = vm_map.find(type);
   if (itr == vm_map.end()) {
      return -1;
   }
   return itr->second->setcode(account);
}

int vm_manager::apply(int type, uint64_t receiver, uint64_t account, uint64_t act) {
   if (is_trusted_account(account)) {

   } else {
      auto itr = vm_map.find(VM_TYPE_IPC);
      if (itr != vm_map.end()) {
         type = VM_TYPE_IPC;
      }
   }
   return local_apply(type, receiver, account, act);
}

int vm_manager::call(uint64_t account, uint64_t func) {
   int type = db_api::get().get_code_type(account);
   auto itr = vm_map.find(type);
   return itr->second->call(account, func);
}

int vm_manager::local_apply(int type, uint64_t receiver, uint64_t account, uint64_t act) {
/*
   if (check_new_version(type, vm_names[type])) {
      load_vm_from_ram(type, vm_names[type]);
   }
*/
#if 0
   if (vm_map[VM_TYPE_NATIVE]->apply(receiver, account, act)) {
      return 1;
   }
#endif

   if (type == 0) { //wasm
      do {
         bool expired = false;
         bool _boosted = false;
         _boosted = is_boost_account(receiver, expired);
         if (!_boosted) {
            unload_account(receiver);
            break;
         }
         if (expired) {
            unload_account(receiver);
            break;
         }

         if (!is_jit_account_activated(receiver)) {
            break;
         }

         auto itr = preload_account_map.find(receiver);
         if (itr != preload_account_map.end()) {
            return itr->second->apply(receiver, account, act);
         } else {
            wlog("executing ${n} by jit", ("n", name(receiver)));
            //accelerating execution by JIT
            //may fall if code is still in loading, if so, execute it in binaryen
            //if code was executed in wavm in one BP but executed in binaryen in the next BP,
            //there is a small probability that the next BP can not execute all the code in one block time
            //at the situation of network overloading, that may cause a fork and BP will lose some revenue
            //a vote on jit loading can solve this problem.
            if (vm_map[VM_TYPE_WAVM]->apply(receiver, account, act)) {
               return 1;
            }
         }
      } while(false);
   }

   auto itr = vm_map.find(type);

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

   fn_get_wasm_vm_api _get_wasm_vm_api = (fn_get_wasm_vm_api)dlsym(itr->second->handle, "vm_get_wasm_api");
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
   fn_wasm_call _wasm_call = (fn_wasm_call)dlsym(itr->second->handle, "vm_call");
   if (_wasm_call == nullptr) {
      return -1;
   }

   return _wasm_call(func.c_str(), _args, args.size());
}

int vm_manager::vm_deinit_all() {
   for (auto itr = vm_map.begin();itr != vm_map.end();itr++) {
      itr->second->vm_deinit();
   }
   return 1;
}

void vm_manager::set_vm_api(struct vm_api* _api) {
   this->api = _api;
}

void vm_manager::add_trusted_account(uint64_t account) {
   trusted_accounts[account] = account;
}

void vm_manager::remove_trusted_account(uint64_t account) {
   auto it = trusted_accounts.find(account);
   if (it != trusted_accounts.end()) {
      trusted_accounts.erase(it);
   }
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

   std::string _wasm_to_wast( const uint8_t* data, uint64_t size, bool strip_names )
   {
      return _wasm_to_wast(data, size);
   }
}
}


extern "C" void vm_unload_account(uint64_t account) {
   vm_manager::get().unload_account(account);
}

extern "C" void vm_deinit_all() {
   vm_manager::get().vm_deinit_all();
}
