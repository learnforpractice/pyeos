#include <appbase/application.hpp>
#include <eosio/net_plugin/net_plugin.hpp>

#include "json.hpp"
#include "net_.hpp"
using namespace appbase;
using namespace eosio;

PyObject* connections_() {
   auto peers = app().get_plugin<net_plugin>().connections();
   return python::json::to_string(fc::variant(peers));
}

string connect_(const string& host) {
   return app().get_plugin<net_plugin>().connect( host );
}

string disconnect_(const string& host) {
   return app().get_plugin<net_plugin>().disconnect( host );
}
PyObject* status_(const string& host) {
   auto s = app().get_plugin<net_plugin>().status( host );
   return python::json::to_string(fc::variant(s));
}
