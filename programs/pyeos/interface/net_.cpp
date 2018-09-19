#include <appbase/application.hpp>
#include <eosio/net_plugin/net_plugin.hpp>

#include "json.hpp"
#include "net_.hpp"
using namespace appbase;
using namespace eosio;

static std::shared_ptr<std::mutex> m(new std::mutex());
static std::shared_ptr<std::condition_variable> cv(new std::condition_variable());

net_plugin& get_net_plugin() {
   abstract_plugin& plugin = app().get_plugin<eosio::net_plugin>();
   return *static_cast<net_plugin*>(&plugin);
}

PyObject* connections_() {
   auto peers = get_net_plugin().connections();
   return python::json::to_string(fc::variant(peers));
}

string connect_(const string& host) {
   std::shared_ptr<string> result = std::make_shared<string>();

   appbase::app().get_io_service().post([result, host](){
      *result = get_net_plugin().connect( host );
      cv->notify_one();
   });

   std::unique_lock<std::mutex> lk(*m);
   cv->wait(lk);
   return *result;
}

string disconnect_(const string& host) {
   std::shared_ptr<string> result = std::make_shared<string>();

   appbase::app().get_io_service().post([result, host](){
      *result = get_net_plugin().disconnect( host );
      cv->notify_one();
   });

   std::unique_lock<std::mutex> lk(*m);
   cv->wait(lk);
   return *result;
}

PyObject* status_(const string& host) {
   auto s = get_net_plugin().status( host );
   return python::json::to_string(fc::variant(s));
}
