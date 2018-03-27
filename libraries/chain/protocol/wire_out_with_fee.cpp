/**
 * DASCOIN!
 *
 */
#include <graphene/chain/protocol/wire_out_with_fee.hpp>

namespace graphene { namespace chain {

  void wire_out_with_fee_operation::validate() const
  {
    FC_ASSERT( fee.amount >= 0 );
    FC_ASSERT( asset_to_wire.amount > 0 );
    FC_ASSERT( !currency_of_choice.empty() );
    FC_ASSERT( !to_address.empty() );
    FC_ASSERT( memo.size() <= DASCOIN_MAXIMUM_INTERNAL_MEMO_LENGTH );
  }

  void wire_out_with_fee_complete_operation::validate() const
  {

  }

  void wire_out_with_fee_reject_operation::validate() const
  {

  }

  void wire_out_with_fee_result_operation::validate() const
  {
  }

} } // namespace graphene::chain
