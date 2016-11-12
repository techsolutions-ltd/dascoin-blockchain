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
    // Registration of accounts:
    account_id_type registrar;
    // Validation of personal information:
    account_id_type pi_validator;
  };

} } // namespace graphene::chain

FC_REFLECT( graphene::chain::chain_authorities,
            (license_issuer)
            (license_authenticator)
            (webasset_issuer)
            (webasset_authenticator)
            (pi_validator)
            (registrar)
          );
