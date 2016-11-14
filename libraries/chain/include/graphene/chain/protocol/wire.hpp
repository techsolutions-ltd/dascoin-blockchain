/**
 * DASCOIN!
 */
#pragma once
#include <graphene/chain/protocol/base.hpp>

namespace graphene { namespace chain {

  struct wire_out_operation : public base_operation
  {
    struct fee_parameters_type {};
    asset fee;   // always zero

    account_id_type account;
    asset asset_to_wire;
    extensions_type extensions;

    account_id_type fee_payer() const { return account; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type& k) const { return 0; }
  };

} }  // namespace graphene::chain

////////////////////////////////
// REFLECTIONS:               //
////////////////////////////////

FC_REFLECT( graphene::chain::wire_out_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::wire_out_operation,
            (account)
            (asset_to_wire)
            (extensions)
          )
