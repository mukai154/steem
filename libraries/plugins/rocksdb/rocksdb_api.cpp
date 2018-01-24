#include "rocksdb_api.hpp"

#include <steem/chain/account_object.hpp>
#include <steem/chain/history_object.hpp>
#include <steem/chain/database.hpp>

#include <steem/plugins/rocksdb/rocksdb_plugin.hpp>
#include <steem/plugins/chain/chain_plugin.hpp>
#include <steem/plugins/json_rpc/json_rpc_plugin.hpp>

#include <future>
#include <thread>

namespace steem {

using chain::account_history_object;
using chain::operation_object;

namespace plugins { namespace rocksdb {

namespace detail {

class api_impl
{
   public:
      api_impl(const steem::plugins::rocksdb::rocksdb_plugin& dataSource) : _dataSource(dataSource),
       _db( appbase::app().get_plugin< steem::plugins::chain::chain_plugin >().db() ) {}

      DECLARE_API_IMPL(
         (get_ops_in_block)
         (get_account_history)
         (test_parallel_get_ops_in_block)
      )

      const rocksdb::rocksdb_plugin& _dataSource;
      chain::database& _db;
};

DEFINE_API_IMPL( api_impl, get_ops_in_block )
{
   get_ops_in_block_return result;
   _dataSource.find_operations_by_block(args.block_num,
      [&result, &args](const tmp_operation_object& op)
      {
         api_operation_object temp(op);
         if( !args.only_virtual || is_virtual_operation( temp.op ) )
            result.ops.emplace_back(std::move(temp));
      }
   );
   return result;
}

DEFINE_API_IMPL( api_impl, get_account_history )
{
   FC_ASSERT( args.limit <= 10000, "limit of ${l} is greater than maxmimum allowed", ("l",args.limit) );
   FC_ASSERT( args.start >= args.limit, "start must be greater than limit" );

   get_account_history_return result;

   _dataSource.find_account_history_data(args.account, args.start, args.limit, 
      [&result, this](unsigned int sequence, const tmp_operation_object& op)
      {
         result.history[sequence] = api_operation_object(op);
      });

   return result;
}

DEFINE_API_IMPL (api_impl, test_parallel_get_ops_in_block)
{
   test_parallel_get_ops_in_block_return retval;

   unsigned int blocksPerThread = args.totalBlockNumber / args.threadNumber;

   std::vector<std::thread> threadGroup;
   unsigned int startBlock = 0;
   std::promise<void> startProcessingTrigger;
   std::shared_future<void> startProcessing(startProcessingTrigger.get_future());
   for(unsigned int i = 1; i <= args.threadNumber; ++i)
   {
      unsigned int endBlock = startBlock + blocksPerThread;
      if(i == args.threadNumber)
         endBlock += args.totalBlockNumber % args.threadNumber;

      threadGroup.emplace_back([i, startBlock, endBlock, &args, &startProcessing, this]()
      {
         ilog("Thread # ${t} waits for green light...", ("t", i));
         startProcessing.wait();
         ilog("Starting working thread # ${t} to process blocks from range: [${s}:${e}]",
            ("t", i)("s", startBlock)("e", endBlock));

         for(unsigned int r = 0; r < args.callRepeatNumber; ++r)
         {
            for(unsigned int b = startBlock; b < endBlock; ++b)
            {
               get_ops_in_block_args a;
               a.block_num = b;
               a.only_virtual = false;
               get_ops_in_block(a);
            }
         }

         ilog("Working thread # ${t} finished processing blocks from range: [${s}:${e}]",
            ("t", i)("s", startBlock)("e", endBlock));
      });

      startBlock += blocksPerThread;
   }

   ilog("Starting worker threads...");
   startProcessingTrigger.set_value();

   for(auto& t : threadGroup)
   {
      if(t.joinable())
        t.join();
   }

   return retval;
}

} // detail

rocksdb_api::rocksdb_api(const rocksdb::rocksdb_plugin& dataSource) : my( new detail::api_impl(dataSource) )
{
   /// intentionally name is same as original account-history api
   JSON_RPC_REGISTER_API( "account_history_api" );
}

rocksdb_api::~rocksdb_api() {}

DEFINE_LOCKLESS_APIS( rocksdb_api,
   (get_ops_in_block)
   (get_account_history)
   (test_parallel_get_ops_in_block)
)

} } } // steem::plugins::rocksdb
