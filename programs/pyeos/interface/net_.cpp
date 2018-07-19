#include <appbase/application.hpp>
#include <eosio/net_plugin/net_plugin.hpp>

#include "json.hpp"
#include "net_.hpp"
using namespace appbase;
using namespace eosio;

net_plugin& get_net_plugin() {
   abstract_plugin& plugin = app().get_plugin<eosio::net_plugin>();
   return *static_cast<net_plugin*>(&plugin);
}

PyObject* connections_() {
   auto peers = get_net_plugin().connections();
   return python::json::to_string(fc::variant(peers));
}

string connect_(const string& host) {
   return get_net_plugin().connect( host );
}

string disconnect_(const string& host) {
   return get_net_plugin().disconnect( host );
}
PyObject* status_(const string& host) {
   auto s = get_net_plugin().status( host );
   return python::json::to_string(fc::variant(s));
}
