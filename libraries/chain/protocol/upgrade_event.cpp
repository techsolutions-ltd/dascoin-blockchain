/**
 * DASCOIN!
 *
 */
#include <graphene/chain/protocol/upgrade.hpp>

namespace graphene { namespace chain {

  void create_upgrade_event_operation::validate() const
  {
    FC_ASSERT( fee.amount >= 0 );
    FC_ASSERT( comment.size() <= DASCOIN_MAXIMUM_INTERNAL_MEMO_LENGTH );
  }

} } // namespace graphene::chain
