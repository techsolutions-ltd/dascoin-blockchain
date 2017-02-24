/**
 * DASCOIN!
 */

#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/db/object.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace graphene { namespace chain {

  ///////////////////////////////
  // OPERATIONS:               //
  ///////////////////////////////

  /**
   * @brief create a license type object
   * @ingroup operations
   *
   * An authorized license administration authority may define licenses to be distributed to users.
   */
  struct create_license_type_operation : public base_operation
  {
     struct fee_parameters_type {};  // No fees are paid for this operation.

     asset fee;
     account_id_type admin;

     string name;
     share_type amount;
     string kind;

     upgrade_multiplier_type balance_multipliers;
     upgrade_multiplier_type requeue_multipliers;
     upgrade_multiplier_type return_multipliers;

     account_id_type fee_payer() const { return admin; }
     void validate() const;
     share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
   * @brief Request a license to be granted an account
   * @ingroup operations
   *
   * An authorized license issuing authority may request a license to be granted to an account.
   * An independent authorized license authentication authority must inspect and approve this request. Upon approval,
   * the license is irredeemably granted to the account.
   */
  struct issue_license_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type license_issuer;  // This MUST be the license issuer authority.

    account_id_type account;                  // The account to benefit the license.
    license_type_id_type license;             // The license to be granted.
    share_type bonus_percentage;              // The bonus multiplier of base license cycles.
    frequency_type frequency;                 // The frequency lock on this license, zero for none.

    extensions_type extensions;

    account_id_type fee_payer() const { return license_issuer; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
   * @brief Approve a license grant to an account
   * @ingroup operations
   *
   * An authorized license authentication authority may approve an existing request made by the license issuing
   * authority. If such a request is approved the license is irrevocably granted to the account.
   */
  struct record_issue_license_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type license_authenticator;  // This MUST be the license issuing authority.

    account_id_type account;                  // The account to benefit the license.
    license_type_id_type license;             // The license to be granted.

    extensions_type   extensions;

    account_id_type fee_payer() const { return license_authenticator; }
    void validate() const { FC_ASSERT( false ); }
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

} }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

FC_REFLECT( graphene::chain::create_license_type_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::create_license_type_operation,
            (fee)
            (admin)
            (name)
            (amount)
            (kind)
            (balance_multipliers)
            (requeue_multipliers)
            (return_multipliers)
          )

FC_REFLECT( graphene::chain::issue_license_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::issue_license_operation,
            (fee)
            (license_issuer)
            (account)
            (license)
            (bonus_percentage)
            (frequency)
            (extensions)
          )

FC_REFLECT( graphene::chain::record_issue_license_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::record_issue_license_operation,
            (fee)
            (license_authenticator)
            (account)
            (license)
            (extensions)
          )

