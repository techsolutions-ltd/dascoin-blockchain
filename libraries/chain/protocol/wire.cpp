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
    FC_ASSERT( memo.size() <= DASCOIN_MAXIMUM_INTERNAL_MEMO_LENGTH );
  }

  void wire_out_complete_operation::validate() const
  {

  }

  void wire_out_reject_operation::validate() const
  {

  }

  void wire_out_result_operation::validate() const
  {
  }

} } // namespace graphene::chain
