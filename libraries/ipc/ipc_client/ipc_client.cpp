#include "ipc_client.hpp"

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


#include <eosio/chain/db_api.hpp>
using namespace eosio::chain;

uint64_t ipc_client::get_receiver() {
   return db_api::get().get_receiver();
}

void ipc_client::require_recipient( uint64_t name ) {

}

void ipc_client::require_auth( uint64_t name ) {

}

void ipc_client::require_auth2( uint64_t name, uint64_t permission ) {

}

void ipc_client::send_inline(char *data, size_t data_len) {

}

void ipc_client::send_context_free_inline(char *data, size_t data_len) {

}

uint64_t ipc_client::publication_time() {
   return 0;
}

uint32_t ipc_client::get_active_producers( uint64_t* producers, uint32_t buffer_size ) {
   return 0;
}

int32_t ipc_client::db_store_i64(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id,  const void* data, uint32_t len) {
   std::string _buffer((char*)data, len);
   rpcclient->db_store_i64(scope, table, payer, id, _buffer);
   return db_api::get().db_find_i64(get_receiver(), scope, table, id);
}

void ipc_client::db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
   uint64_t code;
   uint64_t scope;
   uint64_t _payer;
   uint64_t table;
   uint64_t id;
   db_api::get().db_get_table_i64( itr, code, scope, _payer, table, id );
   std::string _buffer(buffer, buffer_size);
   rpcclient->db_update_i64_ex( scope, payer, table, id, _buffer );
}

void ipc_client::db_remove_i64( int itr ) {
   uint64_t code;
   uint64_t scope;
   uint64_t payer;
   uint64_t table;
   uint64_t id;
   db_api::get().db_get_table_i64( itr, code, scope, payer, table, id );
   rpcclient->db_remove_i64_ex(scope, payer, table, id);
   db_api::get().db_remove_i64_ex(itr);
}

int32_t ipc_client::db_get_i64(int32_t iterator, void* data, uint32_t len) {
   return db_api::get().db_get_i64(iterator, (char*)data, len);
}

int32_t ipc_client::db_get_i64_ex( int itr, uint64_t* primary, char* buffer, size_t buffer_size ) {
   return db_api::get().db_get_i64_ex(itr, *primary, buffer, buffer_size);
}

const char* ipc_client::db_get_i64_exex( int itr, size_t* buffer_size ) {
   return db_api::get().db_get_i64_exex( itr,  buffer_size);
}

int32_t ipc_client::db_next_i64(int32_t iterator, uint64_t* primary) {
   return db_api::get().db_next_i64(iterator, *primary);
}

int32_t ipc_client::db_previous_i64(int32_t iterator, uint64_t* primary) {
   return db_api::get().db_previous_i64(iterator, *primary);
}

int32_t ipc_client::db_find_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id) {
   return db_api::get().db_find_i64(code, scope, table, id);
}

int32_t ipc_client::db_lowerbound_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id) {
   return db_api::get().db_lowerbound_i64(code, scope, table, id);
}

int32_t ipc_client::db_upperbound_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id) {
   return db_api::get().db_upperbound_i64(code, scope, table, id);
}

int32_t ipc_client::db_end_i64(uint64_t code, uint64_t scope, uint64_t table) {
   return db_api::get().db_end_i64(code, scope, table);
}

int32_t ipc_client::check_transaction_authorization( const char* trx_data,     uint32_t trx_size,
                                 const char* pubkeys_data, uint32_t pubkeys_size,
                                 const char* perms_data,   uint32_t perms_size
                               ) {
   return 0;
}

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

int ipc_client::start() {
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
