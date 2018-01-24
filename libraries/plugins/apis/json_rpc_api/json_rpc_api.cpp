#include <steem/plugins/json_rpc_api/json_rpc_api_plugin.hpp>
#include <steem/plugins/json_rpc_api/json_rpc_api.hpp>

#include <steem/chain/database.hpp>
#include <steem/plugins/chain/chain_plugin.hpp>

#include <future>
#include <thread>

namespace steem { namespace plugins { namespace json_rpc {

namespace detail {

class json_rpc_api_impl
{
   public:
      json_rpc_api_impl() : _db( appbase::app().get_plugin< steem::plugins::chain::chain_plugin >().db() ) 
      {
         _rpc_plugin = appbase::app().find_plugin< steem::plugins::json_rpc::json_rpc_plugin >();
      }

      DECLARE_API_IMPL(
         (run_call_in_threads)
      )

      steem::chain::database&    _db;
      json_rpc::json_rpc_plugin* _rpc_plugin = nullptr;
};

DEFINE_API_IMPL( json_rpc_api_impl, run_call_in_threads )
{
   FC_ASSERT(_rpc_plugin != nullptr, "Trying to use unregistered json_rpc plugin");

   std::vector<std::thread> threadGroup;
   std::promise<void> startProcessingTrigger;
   std::shared_future<void> startProcessing(startProcessingTrigger.get_future());
   for(unsigned int i=1; i<= args.thread_number; ++i)
   {
      threadGroup.emplace_back([this, i, &args, &startProcessing]
      {
         ilog("Thread # ${t} waits for green light...", ("t", i));
         startProcessing.wait();
         for(unsigned int r = 0; r < args.call_repeat_number; ++r)
         {
            _rpc_plugin->call( args.call );
         }
      });
   }

   ilog("Starting worker threads...");
   startProcessingTrigger.set_value();

   for(auto& t : threadGroup)
   {
      if(t.joinable())
        t.join();
   }

   run_call_in_threads_return result;
   result.response = "OK";
   return result;
}

} // detail

json_rpc_api::json_rpc_api(): my( new detail::json_rpc_api_impl() )
{
   JSON_RPC_REGISTER_API( STEEM_JSON_RPC_API_PLUGIN_NAME );
}

json_rpc_api::~json_rpc_api() {}

DEFINE_READ_APIS( json_rpc_api,
   (run_call_in_threads)
)

} } } // steem::plugins::json_rpc
