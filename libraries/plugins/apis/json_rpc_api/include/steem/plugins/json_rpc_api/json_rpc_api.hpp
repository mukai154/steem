#pragma once
#include <steem/plugins/json_rpc/utility.hpp>
#include <steem/plugins/database_api/database_api_objects.hpp>

namespace steem { namespace plugins { namespace json_rpc {

namespace detail
{
   class json_rpc_api_impl;
}

struct run_call_in_threads_args
{
   string   call;
   uint32_t call_repeat_number;
   uint32_t thread_number;
};

struct run_call_in_threads_return
{
   string response;
};

class json_rpc_api
{
   public:
      json_rpc_api();
      ~json_rpc_api();

      DECLARE_API(
         /**
          * Runs given rpc call in given number of threads given number of times.
          */
         (run_call_in_threads)
      )

   private:
      std::unique_ptr< detail::json_rpc_api_impl > my;
};

} } } // steem::plugins::json_rpc

FC_REFLECT( steem::plugins::json_rpc::run_call_in_threads_args,
            (call)(call_repeat_number)(thread_number) );

FC_REFLECT( steem::plugins::json_rpc::run_call_in_threads_return,
            (response) );
