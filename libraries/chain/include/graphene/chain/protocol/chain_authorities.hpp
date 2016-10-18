/**
 * DASCOIN!
 */

#pragma once

#include <graphene/chain/protocol/types.hpp>

namespace graphene { namespace chain {

  struct chain_authorities
  {
    account_id_type license_issuer;
    account_id_type license_authenticator;
  };

} } // namespace graphene::chain

FC_REFLECT( graphene::chain::chain_authorities,
            (license_issuer)
            (license_authenticator)
          );
