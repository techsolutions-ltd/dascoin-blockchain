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

    account_id_type get(chain_authority_kind kind) const
    {
       switch (kind)
       {
          case chain_authority_kind::license_issuer:
             return license_issuer;
          case chain_authority_kind::license_authenticator:
             return license_authenticator;
          case chain_authority_kind::webasset_issuer:
             return webasset_issuer;
          case chain_authority_kind::webasset_authenticator:
             return webasset_authenticator;
          case chain_authority_kind::cycle_issuer:
             return cycle_issuer;
          case chain_authority_kind::cycle_authenticator:
             return cycle_authenticator;
          case chain_authority_kind::registrar:
             return registrar;
          case chain_authority_kind::pi_validator:
             return pi_validator;
          case chain_authority_kind::wire_out_handler:
             return wire_out_handler;
          default:
             FC_THROW_EXCEPTION( fc::invalid_arg_exception, "Chain authority ${k} does not exist", ("k", kind) );
       }
    }

    void set(chain_authority_kind kind, account_id_type account_id)
    {
      switch (kind)
      {
       case chain_authority_kind::license_issuer:
         license_issuer = account_id;
         break;
       case chain_authority_kind::license_authenticator:
         license_authenticator = account_id;
         break;
       case chain_authority_kind::webasset_issuer:
         webasset_issuer = account_id;
         break;
       case chain_authority_kind::webasset_authenticator:
         webasset_authenticator = account_id;
         break;
       case chain_authority_kind::cycle_issuer:
         cycle_issuer = account_id;
         break;
       case chain_authority_kind::cycle_authenticator:
         cycle_authenticator = account_id;
         break;
       case chain_authority_kind::registrar:
         registrar = account_id;
         break;
       case chain_authority_kind::pi_validator:
         pi_validator = account_id;
         break;
       case chain_authority_kind::wire_out_handler:
         wire_out_handler = account_id;
         break;
       default:
         FC_THROW_EXCEPTION( fc::invalid_arg_exception, "Chain authority ${k} does not exist", ("k", kind) );
       }
    }
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
