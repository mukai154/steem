#pragma once
#include <steem/plugins/json_rpc/json_rpc_plugin.hpp>

#include <appbase/application.hpp>

#define STEEM_JSON_RPC_API_PLUGIN_NAME "json_rpc_api"


namespace steem { namespace plugins { namespace json_rpc {

using namespace appbase;

class json_rpc_api_plugin : public appbase::plugin< json_rpc_api_plugin >
{
public:
   APPBASE_PLUGIN_REQUIRES(
      (steem::plugins::json_rpc::json_rpc_plugin)
   )

   json_rpc_api_plugin();
   virtual ~json_rpc_api_plugin();

   static const std::string& name() { static std::string name = STEEM_JSON_RPC_API_PLUGIN_NAME; return name; }

   virtual void set_program_options( options_description& cli, options_description& cfg ) override;

   virtual void plugin_initialize( const variables_map& options ) override;
   virtual void plugin_startup() override;
   virtual void plugin_shutdown() override;

   std::shared_ptr< class json_rpc_api > api;
};

} } } // steem::plugins::json_rpc
