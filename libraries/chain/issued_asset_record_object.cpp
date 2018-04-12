/**
 * DASCOIN!
 */

#include <graphene/chain/issued_asset_record_object.hpp>

namespace graphene { namespace chain {

  void issued_asset_record_object::validate() const
  {
    //TODO: add more checks here?
    FC_ASSERT( unique_id.length() > 0 );
  }

} } // namespace graphene::chain