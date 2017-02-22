/**
 * DASCOIN!
 */

#include <graphene/chain/protocol/cycle.hpp>

namespace graphene { namespace chain {

void submit_reserve_cycles_to_queue_operation::validate() const
{

}

void cycle_issue_complete_operation::validate() const
{

}

void deny_submitting_reserve_cycles_to_queue_operation::validate() const
{

}

void submit_cycles_operation::validate() const
{
  FC_ASSERT( amount > 0 );
}

} } // namespace graphene::chain
