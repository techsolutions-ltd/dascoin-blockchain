/**
 * DASCOIN!
 *
 */
#include <graphene/chain/protocol/wire.hpp>

namespace graphene { namespace chain {

  void wire_out_operation::validate() const
  {
    FC_ASSERT( fee.amount >= 0 );
    FC_ASSERT( asset_to_wire.amount > 0 );
  }

  void wire_out_complete_operation::validate() const
  {

  }

} } // namespace graphene::chain
