#include "RpcService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include <thrift/transport/TSocket.h>

#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include <fc/log/logger.hpp>

#include "../../micropython/mpeoslib.h"
#include "readerwriterqueue.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace  ::cpp;

struct ApplyFinish {
   int32_t status;
   std::string errMsg;
};

using namespace moodycamel;

BlockingReaderWriterQueue<Apply> apply_request_queue(1);
BlockingReaderWriterQueue<ApplyFinish> apply_finish_queue(1);

//micropython_interface.cpp
extern "C" int micropython_on_apply(uint64_t receiver, uint64_t account, uint64_t act, char** err);

//mpeoslib.cpp
typedef struct eosapi* (*fn_get_eosapi)();
extern "C" void set_get_eosapi_func(fn_get_eosapi fn);

mp_obj_t uint64_to_string_(uint64_t n);

static struct eosapi s_eosapi;
static RpcServiceClient* rpcclient = nullptr;

//database_api.cpp
extern "C" {
   int mp_action_size();
   int mp_read_action(char* buf, size_t size);
   int mp_is_account(uint64_t account);
   uint64_t mp_get_receiver();

   void mp_db_get_table_i64( int itr, uint64_t *code, uint64_t *scope, uint64_t *payer, uint64_t *table, uint64_t *id);

   //interface/eoslib.hpp
   void db_remove_i64_(int itr);
   int db_get_i64_( int iterator, char* buffer, size_t buffer_size );
   int db_next_i64_( int iterator, uint64_t* primary );
   int db_previous_i64_( int iterator, uint64_t* primary );
   int db_find_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
   int db_lowerbound_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
   int db_upperbound_i64_( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
   int db_end_i64_( uint64_t code, uint64_t scope, uint64_t table );
}

class RpcServiceHandler : virtual public RpcServiceIf {
 public:
  RpcServiceHandler() {
    // Your initialization goes here
  }

  void apply_request(Apply& _return) {
     wlog("apply_request");
     apply_request_queue.wait_dequeue(_return);
  }

  void apply_finish(const int32_t status, const std::string& errMsg) {
     wlog("apply_finish");
     apply_finish_queue.enqueue({status, errMsg});
  }

  void funCall(std::vector<std::string> & _return, const int64_t callTime, const std::string& funCode, const std::map<std::string, std::string> & paramMap) {
    // Your implementation goes here
    printf("funCall\n");
  }

  void read_action(std::string& _return) {
    // Your implementation goes here
    printf("read_action\n");
  }

  int32_t db_store_i64(const int64_t scope, const int64_t table, const int64_t payer, const int64_t id, const std::string& buffer) {
    // Your implementation goes here
    return -1;
  }

  void db_update_i64(const int32_t itr, const int64_t payer, const std::string& buffer) {
    // Your implementation goes here
    printf("db_update_i64\n");
  }

  void db_remove_i64(const int32_t itr) {
    // Your implementation goes here
    printf("db_remove_i64\n");
  }

  void db_get_i64(std::string& _return, const int32_t itr) {
    // Your implementation goes here
    printf("db_get_i64\n");
  }

  void db_next_i64(Result& _return, const int32_t itr) {
    // Your implementation goes here
    printf("db_next_i64\n");
  }

  void db_previous_i64(Result& _return, const int32_t itr) {
    // Your implementation goes here
    printf("db_previous_i64\n");
  }

  int32_t db_find_i64(const int64_t code, const int64_t scope, const int64_t table, const int64_t id) {
    // Your implementation goes here
    return -1;
  }

  int32_t db_lowerbound_i64(const int64_t code, const int64_t scope, const int64_t table, const int64_t id) {
    // Your implementation goes here
     return -1;
  }

  int32_t db_upperbound_i64(const int64_t code, const int64_t scope, const int64_t table, const int64_t id) {
    // Your implementation goes here
     return -1;
  }

  int32_t db_end_i64(const int64_t code, const int64_t scope, const int64_t table) {
    // Your implementation goes here
     return -1;
  }

  void db_update_i64_ex(const int64_t scope, const int64_t payer, const int64_t table, const int64_t id, const std::string& buffer) {
    // Your implementation goes here
    return;
  }

  void db_remove_i64_ex(const int64_t scope, const int64_t payer, const int64_t table, const int64_t id) {
    // Your implementation goes here
    return;
  }

};

static int on_apply(uint64_t receiver, uint64_t account, uint64_t action, char** err, int* len) {
   Apply apply;
   apply.receiver = receiver;
   apply.account = account;
   apply.action = action;

   apply_request_queue.enqueue(apply);
   ApplyFinish finish = {};

   if (!apply_finish_queue.wait_dequeue_timed(finish, std::chrono::milliseconds(5))) {
      std::string errMsg("execution time out!");
      *err = (char*)malloc(errMsg.length());
      memcpy(*err, errMsg.c_str(), errMsg.length());
      *len = errMsg.length();
      return 911;
   }

   *err = (char*)malloc(finish.errMsg.length());
   memcpy(*err, finish.errMsg.c_str(), finish.errMsg.length());
   *len = finish.errMsg.length();
   return finish.status;
}

typedef int (*fn_rpc_apply)(uint64_t receiver, uint64_t code, uint64_t act, char** err, int* len);
extern "C" void rpc_register_apply_call(fn_rpc_apply fn);

extern "C" void rpc_register_cpp_apply_call() {
   rpc_register_apply_call(on_apply);
}

extern "C" int start_server() {
   boost::thread eos( [&]{
      int port = 9090;
     ::apache::thrift::stdcxx::shared_ptr<RpcServiceHandler> handler(new RpcServiceHandler());
     ::apache::thrift::stdcxx::shared_ptr<TProcessor> processor(new RpcServiceProcessor(handler));
     ::apache::thrift::stdcxx::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
     ::apache::thrift::stdcxx::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
     ::apache::thrift::stdcxx::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

     TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
     server.serve();
   } );
  return 0;
}

extern "C" int start_client() {
   if (rpcclient) {
      return 0;
   }

   std::string addr("localhost");
   stdcxx::shared_ptr<TTransport> socket(new TSocket(addr, 9090));
   stdcxx::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
   stdcxx::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
   rpcclient = new RpcServiceClient(protocol);
   while (true) {
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
           std::string err("");
           rpcclient->apply_request(apply);
           char* _err;
           int ret = micropython_on_apply(apply.receiver, apply.account, apply.action, &_err);
           if (ret != 0) {
              err = std::string(_err);
              free(_err);
           }
           rpcclient->apply_finish(ret, err);
         } catch (...) {
            wlog("exception in client, try to reconnect...");
            break;
         }
      }
   }
   return 0;
}



static void mp_require_auth(uint64_t account) {
   printf("+++++++++++++++mp_require_auth\n");
}

static int mp_db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   std::string _buffer(buffer, buffer_size);
   rpcclient->db_store_i64(scope, table, payer, id, _buffer);
   return db_find_i64_(mp_get_receiver(), scope, table, id);
}

static void mp_db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size ) {
   uint64_t code;
   uint64_t scope;
   uint64_t _payer;
   uint64_t table;
   uint64_t id;
   mp_db_get_table_i64(itr, &code, &scope, &_payer, &table, &id);
   std::string _buffer(buffer, buffer_size);
   rpcclient->db_update_i64_ex( scope, payer, table, id, _buffer);
}

static void mp_db_remove_i64( int itr ) {
   uint64_t code;
   uint64_t scope;
   uint64_t payer;
   uint64_t table;
   uint64_t id;
   mp_db_get_table_i64(itr, &code, &scope, &payer, &table, &id);
   rpcclient->db_remove_i64_ex(scope, payer, table, id);
   db_remove_i64_(itr);
}

static int mp_db_get_i64( int itr, char* buffer, size_t buffer_size ) {
   return db_get_i64_(itr, buffer, buffer_size);
}

static int mp_db_next_i64( int itr, uint64_t* primary ) {
   return db_next_i64_(itr, primary);
}

static int mp_db_previous_i64( int itr, uint64_t* primary ) {
   return db_previous_i64_(itr, primary);
}

static int mp_db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_find_i64_(code, scope, table, id);
}

static int mp_db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_lowerbound_i64_(code, scope, table, id);
}

static int mp_db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   return db_upperbound_i64_(code, scope, table, id);
}

static int mp_db_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
   return db_end_i64_(code, scope, table);
}

static struct eosapi* get_eosapi() {
   return &s_eosapi;
}

extern "C" void cpp_init_eosapi() {
   static bool _init = false;
   if (_init) {
      return;
   }
   _init = true;

   s_eosapi.wasm_call = wasm_call_;
//   s_eosapi.string_to_symbol = string_to_symbol;
   s_eosapi.eosio_delay = eosio_delay;
   s_eosapi.now = now;
   s_eosapi.abort_ = abort;
   s_eosapi.eosio_assert = eosio_assert;
   s_eosapi.assert_recover_key = assert_recover_key;

   s_eosapi.recover_key = recover_key;
   s_eosapi.assert_sha256 = assert_sha256;
   s_eosapi.assert_sha1 = assert_sha1;
   s_eosapi.assert_sha512 = assert_sha512;
   s_eosapi.assert_ripemd160 = assert_ripemd160;
   s_eosapi.sha1 = sha1_;
   s_eosapi.sha256 = sha256_;
   s_eosapi.sha512 = sha512_;
   s_eosapi.ripemd160 = ripemd160_;

   s_eosapi.string_to_uint64_ = string_to_uint64_;
   s_eosapi.uint64_to_string_ = uint64_to_string_;

   s_eosapi.pack_ = pack_;
   s_eosapi.unpack_ = unpack_;

   s_eosapi.action_size = mp_action_size;
   s_eosapi.read_action = mp_read_action;

   s_eosapi.require_auth = mp_require_auth;

   s_eosapi.db_store_i64 = mp_db_store_i64;
   s_eosapi.db_update_i64 = mp_db_update_i64;
   s_eosapi.db_remove_i64 = mp_db_remove_i64;
   s_eosapi.db_get_i64 = mp_db_get_i64;
//   s_eosapi.db_get_i64_ex = mp_db_get_i64_ex;
   s_eosapi.db_next_i64 = mp_db_next_i64;
   s_eosapi.db_previous_i64 = mp_db_previous_i64;
   s_eosapi.db_find_i64 = mp_db_find_i64;
   s_eosapi.db_lowerbound_i64 = mp_db_lowerbound_i64;
   s_eosapi.db_upperbound_i64 = mp_db_upperbound_i64;
   s_eosapi.db_end_i64 = mp_db_end_i64;

   s_eosapi.is_account = mp_is_account;


//mpeoslib.cpp
   s_eosapi.split_path = split_path;

   set_get_eosapi_func(get_eosapi);
}
