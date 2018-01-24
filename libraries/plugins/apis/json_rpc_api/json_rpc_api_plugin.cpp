#include <steem/plugins/json_rpc_api/json_rpc_api_plugin.hpp>
#include <steem/plugins/json_rpc_api/json_rpc_api.hpp>


namespace steem { namespace plugins { namespace json_rpc {

json_rpc_api_plugin::json_rpc_api_plugin() {}
json_rpc_api_plugin::~json_rpc_api_plugin() {}

void json_rpc_api_plugin::set_program_options( options_description& cli, options_description& cfg ) {}

void json_rpc_api_plugin::plugin_initialize( const variables_map& options )
{
   api = std::make_shared< json_rpc_api >();
}

void json_rpc_api_plugin::plugin_startup() {}
void json_rpc_api_plugin::plugin_shutdown() {}

} } } // steem::plugins::json_rpc
