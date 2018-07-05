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


using namespace eosio;
using namespace eosio::chain;
namespace bio = boost::iostreams;


static const int TYPE_BINARYEN = 0;
static const int TYPE_PY = 1;
static const int TYPE_ETH = 2;
static const int TYPE_WAVM = 3;
static const int TYPE_IPC = 4;

namespace eosio {
namespace chain {
   typedef void (*fn_register_vm_api)(struct vm_api* api);
   void register_vm_api(void* handle) {
//      fn_register_vm_api _register_vm_api = (fn_register_vm_api)dlsym(handle, "vm_register_api");
//      _register_vm_api(&_vm_api);
   }
   int  wasm_to_wast( const uint8_t* data, size_t size, uint8_t* wast, size_t wast_size );
}
}

#define WAVM_VM_START_INDEX (0x10000)

typedef void (*fn_on_boost_account)(void* v, uint64_t account);
void visit_boost_account(fn_on_boost_account fn, void* param);

typedef struct vm_py_api* (*fn_get_py_vm_api)();
typedef struct vm_wasm_api* (*fn_get_wasm_vm_api)();
typedef uint64_t (*fn_wasm_call)(const char* act, uint64_t* args, int argc);
bool is_boost_account(uint64_t account, bool& expired);

void _on_boost_account(void* v, uint64_t account) {
   vm_manager* mngr = (vm_manager*)v;
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
      N(vm.wasm.1),
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
#ifdef DEBUG
   "../libs/libvm_wasm_binaryend" DYLIB_SUFFIX,
   "../libs/libvm_py-1d" DYLIB_SUFFIX,
   "../libs/libvm_ethd" DYLIB_SUFFIX,
   "../libs/libvm_wasm_wavm-0d" DYLIB_SUFFIX
   "../libs/libvm_api_ipcd" DYLIB_SUFFIX
#else
   "../libs/libvm_wasm_binaryen" DYLIB_SUFFIX,
   "../libs/libvm_py-1" DYLIB_SUFFIX,
   "../libs/libvm_eth" DYLIB_SUFFIX,
   "../libs/libvm_wasm_wavm-0" DYLIB_SUFFIX
   "../libs/libvm_api_ipc" DYLIB_SUFFIX
#endif
};

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
      wlog("load ${n1} cost ${n2}", ("n1", name(account))("n2", get_microseconds()-start));
   }
private:
   uint64_t account;
   uint64_t start;
};

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

   char _path[128];
#ifdef DEBUG
   const char* _format = "../libs/libvm_wasm_wavm-%dd" DYLIB_SUFFIX;
#else
   const char* _format = "../libs/libvm_wasm_wavm-%d" DYLIB_SUFFIX;
#endif

   for (int i=1;i<=10;i++) {
      snprintf(_path, sizeof(_path), _format, i);
      load_vm_from_path(WAVM_VM_START_INDEX|i, _path);
   }

   visit_boost_account(_on_boost_account, this);

   auto itr = vm_map.find(3);
   if (itr != vm_map.end()) {
      if (db_api::get().is_account(N(eosio.token))) {
         auto t = time_counter(N(eosio.token));
         itr->second->preload(N(eosio.token));
      }
      if (db_api::get().is_account(N(eosio))) {
         auto t = time_counter(N(eosio));
         itr->second->preload(N(eosio));
      }
   }

   if (boost_accounts.size() == 0) {
      return true;
   }

   boost::thread_group g;

   for (int i=1;i<=6;i++) {//TODO: 10 --> number of CPU cores
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
   return true;
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

   fn_setcode setcode = (fn_setcode)dlsym(handle, "vm_setcode");
   if (setcode == NULL) {
      return 0;
   }

   fn_apply apply = (fn_apply)dlsym(handle, "vm_apply");
   if (apply == NULL) {
      return 0;
   }

   fn_preload preload = (fn_preload)dlsym(handle, "vm_preload");
   /*
   if (preload == NULL) {
      return 0;
   }
   */

   fn_unload unload = (fn_unload)dlsym(handle, "vm_unload");

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
   calls->preload = preload;
   calls->unload = unload;

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

   fn_setcode setcode = (fn_setcode)dlsym(handle, "vm_setcode");
   if (setcode == NULL) {
      return 0;
   }

   fn_apply apply = (fn_apply)dlsym(handle, "vm_apply");
   if (apply == NULL) {
      return 0;
   }

   fn_preload preload = (fn_preload)dlsym(handle, "vm_preload");
   /*
   if (preload == NULL) {
      return 0;
   }
   */

   fn_unload unload = (fn_unload)dlsym(handle, "vm_unload");


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
   calls->preload = preload;
   calls->unload = unload;

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

bool vm_manager::is_trusted_account(uint64_t account) {
   return true;
}

void vm_manager::set_vm_api(struct vm_api* _api) {
   this->api = _api;
}

struct vm_api* vm_manager::get_vm_api() {
   return this->api;
}

int vm_manager::setcode(int type, uint64_t account) {
/*
   if (check_new_version(type, vm_names[type])) {
      load_vm(type, vm_names[type]);
   }
*/
   if (is_trusted_account(account)) {
   } else {
      type = TYPE_IPC;
   }
   auto itr = vm_map.find(type);
   if (itr == vm_map.end()) {
      return -1;
   }
   return itr->second->setcode(account);
}

int vm_manager::apply(int type, uint64_t receiver, uint64_t account, uint64_t act) {
   if (is_trusted_account(account)) {
      return local_apply(type, receiver, account, act);
   } else {
      auto itr = vm_map.find(TYPE_IPC);
      if (itr == vm_map.end()) {
         return -1;
      }
      return itr->second->apply(receiver, account, act);
   }
}

int vm_manager::local_apply(int type, uint64_t receiver, uint64_t account, uint64_t act) {
/*
   if (check_new_version(type, vm_names[type])) {
      load_vm(type, vm_names[type]);
   }
*/
   if (type == 0) { //wasm
      do {
         if (receiver == N(eosio) || receiver == N(eosio.token)) {
            type = 3;
            break;
         }
         bool expired = false;
         bool _boosted = false;
         _boosted = is_boost_account(receiver, expired);
         if (!_boosted) {
            break;
         }
         if (expired) {
            unload_account(receiver);
            break;
         } else {
            auto itr = preload_account_map.find(receiver);
            if (itr != preload_account_map.end()) {
               return itr->second->apply(receiver, account, act);
            } else {
               wlog("executing ${n} by jit", ("n", name(receiver)));
               type = 3;//accelerating execution by JIT
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


extern "C" void vm_unload_account(uint64_t account) {
   vm_manager::get().unload_account(account);
}

