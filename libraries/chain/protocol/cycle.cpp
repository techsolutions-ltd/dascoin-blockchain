/**
 * DASCOIN!
 */

#include <graphene/chain/protocol/cycle.hpp>

namespace graphene { namespace chain {

void submit_reserve_cycles_to_queue_operation::validate() const
{
  FC_ASSERT( amount > 0, "Must submit a non-zero value" );
  FC_ASSERT( frequency_lock > 0, "Must submit a non-zero value" );
  FC_ASSERT( comment.length() <= DASCOIN_MAX_COMMENT_LENGTH );
}

void submit_cycles_to_queue_operation::validate() const
{
  FC_ASSERT( amount > 0 );
}

void update_queue_parameters_operation::validate() const
{
  if ( reward_interval_time_seconds.valid() )
    FC_ASSERT( *reward_interval_time_seconds > GRAPHENE_MIN_BLOCK_INTERVAL, 
              "Must be greater than the minimal block interval" 
    );
  if ( dascoin_reward_amount.valid() )
    FC_ASSERT( *dascoin_reward_amount > DASCOIN_MIN_DASCOIN_REWARD_AMOUNT,
               "Reward amount too small"
             );
}

void update_global_frequency_operation::validate() const
{
  FC_ASSERT( frequency > 0 );  // TODO: maximum frequency?
}

} } // namespace graphene::chain
