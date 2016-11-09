/**
 * DASCOIN!
 */
#include <graphene/chain/protocol/personal_identity.hpp>

namespace graphene { namespace chain {

void update_pi_limits_operation::validate() const
{
  if ( new_limits.valid() )
    FC_ASSERT( (*new_limits).size() < limit_kind::LIMIT_KIND_COUNT );
}

} }  // namespace graphene::chain
