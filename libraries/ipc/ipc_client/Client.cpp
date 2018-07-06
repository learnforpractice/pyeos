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

#define FC_CATCH_EXC( err )  \
   catch( const boost::interprocess::bad_alloc& ) {\
      err = "bad_alloc";\
   } catch( fc::exception& er ) { \
      err = er.to_detail_string(); \
   } catch( const std::exception& e ) {  \
      fc::exception fce( \
                FC_LOG_MESSAGE( warn, "rethrow ${what}: ", ("what",e.what())), \
                fc::std_exception_code,\
                BOOST_CORE_TYPEID(e).name(), \
                e.what() ) ; \
      err = fce.to_detail_string(); \
   } catch( ... ) {  \
      fc::unhandled_exception e( \
                FC_LOG_MESSAGE( warn, "rethrow"), \
                std::current_exception() ); \
      err = e.to_detail_string(); \
   }

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
            Apply apply;
            string err;
            int ret;
            wlog("+++++++++++apply_request");
           rpcclient->apply_request(apply);
           wlog("+++++++++++apply_request return");
           try {
              ret = vm_manager::get().apply(0, apply.receiver, apply.account, apply.action);
           } FC_CATCH_EXC(err);
           wlog("+++++++++++++apply_finish ret ${n1} err ${n2}", ("n1", ret)("n2", err));
           rpcclient->apply_finish(ret, err);
           wlog("+++++++++++++apply_finish return");
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

int main(int argc, char** argv) {
   wlog("hello,world from ipc client");
   eosio::chain::vm_manager_init();
   start_client();
   return 0;
}

