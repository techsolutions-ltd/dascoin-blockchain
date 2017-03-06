/**
 * DASCOIN!
 */

#include <graphene/chain/license_objects.hpp>

namespace graphene { namespace chain {

  void license_type_object::validate() const
  {
    FC_ASSERT( name.size() >= GRAPHENE_MIN_ACCOUNT_NAME_LENGTH );
    FC_ASSERT( name.size() <= GRAPHENE_MAX_ACCOUNT_NAME_LENGTH );
  }

} } // namespace graphene::chain
