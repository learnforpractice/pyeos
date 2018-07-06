#include <eosiolib_native/vm_api.h>

#include <boost/process.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <memory>

using namespace boost::process;

std::unique_ptr<boost::thread> client_monitor_thread;
std::unique_ptr<boost::thread> server_thread;

extern "C" int start_server(const char* ipc_path);

void vm_init() {
   client_monitor_thread.reset(new boost::thread([]{
         ipstream pipe_stream;
         child c("../libraries/ipc/ipc_client/ipc_client", std_out > pipe_stream);
         std::string line;
         while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()) {
            std::cerr << line << std::endl;
         }
         c.wait();
   }));

   server_thread.reset(new boost::thread([]{
         start_server("/tmp/pyeos.ipc");
   }));
}

void vm_deinit() {

}

void vm_register_api(struct vm_api* api) {

}

int vm_setcode(uint64_t account) {
   return -1;
}

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   return -1;
}

uint64_t vm_call(const char* act, uint64_t* args, int argc) {
   return 0;
}

int vm_preload(uint64_t account) {
   return -1;
}

int vm_unload(uint64_t account) {
   return -1;
}
