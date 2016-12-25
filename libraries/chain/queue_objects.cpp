/**
 * DASCOIN!
 */
#include <graphene/chain/queue_objects.hpp>

namespace graphene { namespace chain {

  void reward_queue_object::validate() const
  {
    FC_ASSERT( amount > 0 );
    FC_ASSERT( frequency > 0 );
  }

} }  // namespace graphene::chain
