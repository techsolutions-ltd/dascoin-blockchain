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
  FC_ASSERT( frequency > 0 );
  // Comment's format needs to be x.yy.z
  FC_ASSERT( comment.length() >= 6 && comment.length() <= DASCOIN_MAX_COMMENT_LENGTH );
}

void submit_cycles_to_queue_by_license_operation::validate() const
{
  FC_ASSERT( amount > 0, "Must submit a non-zero value" );
  FC_ASSERT( comment.length() <= DASCOIN_MAX_COMMENT_LENGTH );
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
  FC_ASSERT( comment.length() <= DASCOIN_MAX_COMMENT_LENGTH );
}

void issue_free_cycles_operation::validate() const
{
  FC_ASSERT( amount > 0 );  // TODO: maximum amount?
  FC_ASSERT( origin < cycle_origin_kind::CYCLE_ORIGIN_KIND_COUNT );
  FC_ASSERT( comment.length() <= DASCOIN_MAX_COMMENT_LENGTH );
}

void issue_cycles_to_license_operation::validate() const
{
  FC_ASSERT( amount > 0 );
  FC_ASSERT( origin.length() <= DASCOIN_MAX_COMMENT_LENGTH );
  FC_ASSERT( comment.length() <= DASCOIN_MAX_COMMENT_LENGTH );
}

void purchase_cycle_asset_operation::validate() const
{
  FC_ASSERT( amount > 0 );
  FC_ASSERT( frequency > 0 );
  FC_ASSERT( expected_amount > 0 );
  // Make sure we get an integer value of cycles:
  FC_ASSERT( (amount * frequency) % (DASCOIN_DEFAULT_ASSET_PRECISION * DASCOIN_FREQUENCY_PRECISION) == 0 );
  FC_ASSERT( (amount * frequency) / (DASCOIN_DEFAULT_ASSET_PRECISION * DASCOIN_FREQUENCY_PRECISION) == expected_amount );
}

void transfer_cycles_from_licence_to_wallet_operation::validate() const
{
  FC_ASSERT ( amount > 0 );
}

} } // namespace graphene::chain
