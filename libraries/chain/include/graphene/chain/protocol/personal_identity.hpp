/**
 * DASCOIN!
 */
#pragma once
#include <graphene/chain/protocol/base.hpp>

namespace graphene { namespace chain {

///////////////////////////////
// Operations:               //
///////////////////////////////

  struct update_pi_limits_operation : public base_operation
  {
    struct fee_parameters_type {};

    asset fee;
    account_id_type pi_validator;  // This is the id of the validation authority that assigns the pi and levels.
    uint8_t level;  // The new level
    optional<flat_set<share_type>> new_limits;

    extensions_type extensions;

    account_id_type fee_payer()const { return pi_validator; }
    void validate()const;
    share_type calculate_fee(const fee_parameters_type& k)const { return 0; }
  };

} }  // namespace graphene::chain

////////////////////////////////
// Reflections:               //
////////////////////////////////

FC_REFLECT( graphene::chain::update_pi_limits_operation::fee_parameters_type,  )

FC_REFLECT( graphene::chain::update_pi_limits_operation,
            (fee)
            (pi_validator)
            (level)
            (new_limits)
            (extensions)
          )
