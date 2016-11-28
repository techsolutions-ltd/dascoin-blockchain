/**
 * DASCOIN!
 */

#pragma once

#include <graphene/chain/protocol/types.hpp>

namespace graphene { namespace chain {

  struct chain_authorities
  {
    // License related:
    account_id_type license_issuer;
    account_id_type license_authenticator;
    // Webasset related:
    account_id_type webasset_issuer;
    account_id_type webasset_authenticator;
    // Cycle related:
    account_id_type cycle_issuer;
    account_id_type cycle_authenticator;
    // Registration of accounts:
    account_id_type registrar;
    // Validation of personal information:
    account_id_type pi_validator;
    // Handling of wire_out payments:
    account_id_type wire_out_handler;
  };

} } // namespace graphene::chain

FC_REFLECT( graphene::chain::chain_authorities,
            (license_issuer)
            (license_authenticator)
            (webasset_issuer)
            (webasset_authenticator)
            (cycle_issuer)
            (cycle_authenticator)
            (registrar)
            (pi_validator)
            (wire_out_handler)
          );
