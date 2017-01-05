/**
 * DASCOIN!
 */

#include <graphene/chain/protocol/cycle.hpp>

namespace graphene { namespace chain {

void cycle_issue_request_operation::validate() const
{

}

void cycle_issue_complete_operation::validate() const
{

}

void cycle_issue_deny_operation::validate() const
{

}

void submit_cycles_operation::validate() const
{
  FC_ASSERT( amount > 0 );
}

} } // namespace graphene::chain
