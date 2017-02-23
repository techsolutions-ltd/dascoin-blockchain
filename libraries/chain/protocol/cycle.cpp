/**
 * DASCOIN!
 */

#include <graphene/chain/protocol/cycle.hpp>

namespace graphene { namespace chain {

void submit_reserve_cycles_to_queue_operation::validate() const
{
  FC_ASSERT( amount > 0, "Must submit a non-zero value" );
  FC_ASSERT( frequency_lock > 0, "Must submit a non-zero value" );
}

void cycle_issue_complete_operation::validate() const
{

}

void deny_submit_reserve_cycles_to_queue_operation::validate() const
{

}

void submit_cycles_to_queue_operation::validate() const
{
  FC_ASSERT( amount > 0 );
}

} } // namespace graphene::chain
