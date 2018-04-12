/**
 * DASCOIN!
 */
#include <graphene/chain/queue_objects.hpp>

namespace graphene { namespace chain {

  void reward_queue_object::validate() const
  {
    FC_ASSERT( amount > 0 );
    FC_ASSERT( frequency > 0 );
    FC_ASSERT( comment.length() <= DASCOIN_MAX_COMMENT_LENGTH );
  }

} }  // namespace graphene::chain
