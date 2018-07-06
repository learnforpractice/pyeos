#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSocket.h>

#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

#include <fc/log/logger.hpp>

#include <softfloat.hpp>
#include <eosiolib_native/vm_api.h>

#include <eosio/chain/db_api.hpp>
#include "thrift/blockingconcurrentqueue.h"
#include "thrift/readerwriterqueue.h"
#include "thrift/RpcService.h"

#include <vm_manager.hpp>

using namespace eosio::chain;

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;


using namespace  ::cpp;




static RpcServiceClient* rpcclient = nullptr;


extern "C" int start_client() {
   if (rpcclient) {
      return 0;
   }

   while (true) {
      if (rpcclient) {
         delete rpcclient;
      }
      std::string addr("localhost");
//      stdcxx::shared_ptr<TTransport> socket(new TSocket(addr, 9191));
      stdcxx::shared_ptr<TTransport> socket(new TSocket("/tmp/pyeos.ipc"));
      stdcxx::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      stdcxx::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      rpcclient = new RpcServiceClient(protocol);

      while (true) {
         try {
            transport->open();
            break;
         } catch (...) {
            wlog("exception in transport->open()!");
         }
         boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
      }

      wlog("transport->open() successfully returned.");

      while (true) {
         try {
#if 0
            Apply apply;
           std::string err("");
           wlog("+++++++++++apply_request");
           rpcclient->apply_request(apply);
           wlog("+++++++++++apply_request return");
           char* _err;
           int ret = micropython_on_apply(apply.receiver, apply.account, apply.action, &_err);
           if (ret != 0) {
              err = std::string(_err);
              free(_err);
           }
           wlog("+++++++++++++apply_finish");
           rpcclient->apply_finish(ret, err);
           wlog("+++++++++++++apply_finish return");
#endif
         } catch (TTransportException& ex) {
            wlog("+++++++++++=exception ocurr when executing code: ${n}", ("n", ex.what()));
            break;
         }
      }
   }
   return 0;
}

namespace eosio {
namespace chain {
   void vm_manager_init();
}
}

extern "C" bool is_server_mode() {
   return false;
}

int main(int argc, char** argv) {
   wlog("hello,world from ipc client");
   eosio::chain::vm_manager_init();
   start_client();
   return 0;
}

