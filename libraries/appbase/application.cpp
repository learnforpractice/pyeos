#include <appbase/application.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio/signal_set.hpp>

#include <iostream>
#include <fstream>

namespace appbase {

namespace bpo = boost::program_options;
using bpo::options_description;
using bpo::variables_map;
using std::cout;

class application_impl {
   public:
      application_impl():_app_options("Application Options"){
      }
      const variables_map*    _options = nullptr;
      options_description     _app_options;
      options_description     _cfg_options;

      bfs::path               _data_dir{"data-dir"};
      bfs::path               _config_dir{"config-dir"};
      bfs::path               _logging_conf{"logging.json"};

      uint64_t                _version;
      bool _debug = false;
      bool _rpc = false;
      bool _client = false;
      bool _server = false;
      bool _interactive = false;
      };

application::application()
:my(new application_impl()){
   io_serv = std::make_shared<boost::asio::io_service>();
}

application::~application() { }

void application::set_version(uint64_t version) {
  my->_version = version;
}

uint64_t application::version() const {
  return my->_version;
}

void application::set_default_data_dir(const bfs::path& data_dir) {
  my->_data_dir = data_dir;
}

void application::set_default_config_dir(const bfs::path& config_dir) {
  my->_config_dir = config_dir;
}

bfs::path application::get_logging_conf() const {
  return my->_logging_conf;
}

void application::startup() {
   try {
      for (auto plugin : initialized_plugins)
         plugin->startup();
   } catch(...) {
      shutdown();
      throw;
   }
}

application& application::instance() {
   static application _app;
   return _app;
}
application& app() { return application::instance(); }


void application::set_program_options()
{
   for(auto& plug : plugins) {
      boost::program_options::options_description plugin_cli_opts("Command Line Options for " + plug.second->name());
      boost::program_options::options_description plugin_cfg_opts("Config Options for " + plug.second->name());
      plug.second->set_program_options(plugin_cli_opts, plugin_cfg_opts);
      if(plugin_cfg_opts.options().size()) {
         my->_app_options.add(plugin_cfg_opts);
         my->_cfg_options.add(plugin_cfg_opts);
      }
      if(plugin_cli_opts.options().size())
         my->_app_options.add(plugin_cli_opts);
   }

   options_description app_cfg_opts( "Application Config Options" );
   options_description app_cli_opts( "Application Command Line Options" );
   app_cfg_opts.add_options()
         ("debug", bpo::bool_switch()->notifier([this](bool e){my->_debug = e;}), "Enable debugging.")
         ("rpc-server", bpo::bool_switch()->notifier([this](bool e){my->_server = e;}), "Setup a eosnode in rpc server mode.")
         ("server", bpo::bool_switch()->notifier([this](bool e){;}), "Setup a eosnode in rpc server mode.")
         ("rpc-client", bpo::bool_switch()->notifier([this](bool e){my->_client = e;}), "Setup a eosnode in rpc client mode.")
         ("interactive,i", bpo::bool_switch()->notifier([this](bool e){my->_interactive = e;}), "Enter in an interactive console.")
         ("plugin", bpo::value< vector<string> >()->composing(), "Plugin(s) to enable, may be specified multiple times");

   app_cli_opts.add_options()
         ("help,h", "Print this help message and exit.")
         ("version,v", "Print version information.")
         ("print-default-config", "Print default configuration template")
         ("data-dir,d", bpo::value<std::string>(), "Directory containing program runtime data")
         ("config-dir", bpo::value<std::string>(), "Directory containing configuration files such as config.ini")
         ("config,c", bpo::value<std::string>()->default_value( "config.ini" ), "Configuration file name relative to config-dir")
         ("logconf,l", bpo::value<std::string>()->default_value( "logging.json" ), "Logging configuration file name/path for library users");

   my->_cfg_options.add(app_cfg_opts);
   my->_app_options.add(app_cfg_opts);
   my->_app_options.add(app_cli_opts);
}

bool application::initialize_impl(int argc, char** argv, vector<abstract_plugin*> autostart_plugins) {
   set_program_options();

   bpo::variables_map options;
   bpo::store(bpo::parse_command_line(argc, argv, my->_app_options), options);

   if( options.count( "help" ) ) {
      cout << my->_app_options << std::endl;
      return false;
   }

   if( options.count( "version" ) ) {
      cout << my->_version << std::endl;
      return false;
   }

   if( options.count( "print-default-config" ) ) {
      print_default_config(cout);
      return false;
   }

   if( options.count( "data-dir" ) ) {
      // Workaround for 10+ year old Boost defect
      // See https://svn.boost.org/trac10/ticket/8535
      // Should be .as<bfs::path>() but paths with escaped spaces break bpo e.g.
      // std::exception::what: the argument ('/path/with/white\ space') for option '--data-dir' is invalid
      auto workaround = options["data-dir"].as<std::string>();
      bfs::path data_dir = workaround;
      if( data_dir.is_relative() )
         data_dir = bfs::current_path() / data_dir;
      my->_data_dir = data_dir;
   }

   if( options.count( "config-dir" ) ) {
      auto workaround = options["config-dir"].as<std::string>();
      bfs::path config_dir = workaround;
      if( config_dir.is_relative() )
         config_dir = bfs::current_path() / config_dir;
      my->_config_dir = config_dir;
   }

   auto workaround = options["logconf"].as<std::string>();
   bfs::path logconf = workaround;
   if( logconf.is_relative() )
      logconf = my->_config_dir / logconf;
   my->_logging_conf = logconf;

   workaround = options["config"].as<std::string>();
   bfs::path config_file_name = workaround;
   if( config_file_name.is_relative() )
      config_file_name = my->_config_dir / config_file_name;

   if(!bfs::exists(config_file_name)) {
      if(config_file_name.compare(my->_config_dir / "config.ini") != 0)
      {
         cout << "Config file " << config_file_name << " missing." << std::endl;
         return false;
      }
      write_default_config(config_file_name);
   }

   bpo::store(bpo::parse_config_file<char>(config_file_name.make_preferred().string().c_str(),
                                           my->_cfg_options, true), options);

   if(options.count("plugin") > 0)
   {
      auto plugins = options.at("plugin").as<std::vector<std::string>>();
      for(auto& arg : plugins)
      {
         vector<string> names;
         boost::split(names, arg, boost::is_any_of(" \t,"));
         for(const std::string& name : names)
            get_plugin(name).initialize(options);
      }
   }
   for (auto plugin : autostart_plugins)
      if (plugin != nullptr && plugin->get_state() == abstract_plugin::registered)
         plugin->initialize(options);

   bpo::notify(options);

   return true;
}

void application::shutdown() {
   for(auto ritr = running_plugins.rbegin();
       ritr != running_plugins.rend(); ++ritr) {
      (*ritr)->shutdown();
   }
   for(auto ritr = running_plugins.rbegin();
       ritr != running_plugins.rend(); ++ritr) {
      plugins.erase((*ritr)->name());
   }
   running_plugins.clear();
   initialized_plugins.clear();
   plugins.clear();
   io_serv.reset();
}

void application::quit() {
   io_serv->stop();
}

bool application::debug_mode() const {
   return my->_debug;
}

bool application::client_mode() const {
   return my->_client;
}

void application::set_client_mode(bool client_mode) {
   my->_client = client_mode;
}

bool application::rpc_enabled() const {
   return my->_rpc;
}

bool application::interactive_mode() const {
   return my->_interactive;
}

void application::exec() {

   std::shared_ptr<boost::asio::signal_set> sigint_set(new boost::asio::signal_set(*io_serv, SIGINT));
   sigint_set->async_wait([sigint_set,this](const boost::system::error_code& err, int num) {
     quit();
     sigint_set->cancel();
   });

   std::shared_ptr<boost::asio::signal_set> sigterm_set(new boost::asio::signal_set(*io_serv, SIGTERM));
   sigterm_set->async_wait([sigterm_set,this](const boost::system::error_code& err, int num) {
     quit();
     sigterm_set->cancel();
   });

   io_serv->run();

   shutdown(); /// perform synchronous shutdown
}

void application::write_default_config(const bfs::path& cfg_file) {
   if(!bfs::exists(cfg_file.parent_path()))
      bfs::create_directories(cfg_file.parent_path());

   std::ofstream out_cfg( bfs::path(cfg_file).make_preferred().string());
   print_default_config(out_cfg);
   out_cfg.close();
}

void application::print_default_config(std::ostream& os) {
   std::map<std::string, std::string> option_to_plug;
   for(auto& plug : plugins) {
      boost::program_options::options_description plugin_cli_opts;
      boost::program_options::options_description plugin_cfg_opts;
      plug.second->set_program_options(plugin_cli_opts, plugin_cfg_opts);

      for(const boost::shared_ptr<bpo::option_description>& opt : plugin_cfg_opts.options())
         option_to_plug[opt->long_name()] = plug.second->name();
   }

   for(const boost::shared_ptr<bpo::option_description> od : my->_cfg_options.options())
   {
      if(!od->description().empty()) {
         os << "# " << od->description();
         std::map<std::string, std::string>::iterator it;
         if((it = option_to_plug.find(od->long_name())) != option_to_plug.end())
            os << " (" << it->second << ")";
         os << std::endl;
      }
      boost::any store;
      if(!od->semantic()->apply_default(store))
         os << "# " << od->long_name() << " = " << std::endl;
      else {
         auto example = od->format_parameter();
         if(example.empty())
            // This is a boolean switch
            os << od->long_name() << " = " << "false" << std::endl;
         else {
            // The string is formatted "arg (=<interesting part>)"
            example.erase(0, 6);
            example.erase(example.length()-1);
            os << od->long_name() << " = " << example << std::endl;
         }
      }
      os << std::endl;
   }
}

abstract_plugin* application::find_plugin(const string& name)const
{
   auto itr = plugins.find(name);
   if(itr == plugins.end()) {
      return nullptr;
   }
   return itr->second.get();
}

abstract_plugin& application::get_plugin(const string& name)const {
   auto ptr = find_plugin(name);
   if(!ptr)
      BOOST_THROW_EXCEPTION(std::runtime_error("unable to find plugin: " + name));
   return *ptr;
}

bfs::path application::data_dir() const {
   return my->_data_dir;
}

bfs::path application::config_dir() const {
   return my->_config_dir;
}

} /// namespace appbase

extern "C" void set_client_mode(int client_mode) {
   appbase::app().set_client_mode(client_mode);
}
