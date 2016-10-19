/**
 * DASCOIN!
 */

#pragma once

#include <graphene/chain/protocol/types.hpp>

namespace graphene { namespace chain {

  struct chain_authorities
  {
    // License realated:
    account_id_type license_issuer;
    account_id_type license_authenticator;
    // Registration of accounts:
    account_id_type registrar;
  };

} } // namespace graphene::chain

FC_REFLECT( graphene::chain::chain_authorities,
            (license_issuer)
            (license_authenticator)
            (registrar)
          );
